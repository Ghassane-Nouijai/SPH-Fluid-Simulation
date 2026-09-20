#pragma once

#include <optional>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <algorithm>

#include "Scene.h"
#include "PhysicsWorld.h"
#include "Sphere.h"
#include "Rectangle.h"
#include "SimObject.h"
#include "PhysicsObject.h"
#include "Renderer.h"
#include "Shader.h"
#include "Camera.h"
#include "InputProcesses.h"

#include "ParticleSandbox.h"

struct WindowConfig
{
	unsigned int width = 1200;
	unsigned int height = 900;
	const char* title = "Simulation";
};

class Application
{
public:
	explicit Application(const WindowConfig& cfg = {})
		: m_Config(cfg), m_Camera(glm::vec3(0.0f, 5.0f, 25.0f))
	{
		Init();

		m_Shader.emplace("Material.shader");
		// Separate shader for particles: the vertex stage reads per-instance
		// position/scale from a vertex attribute instead of a per-draw
		// 'model' uniform (see ParticleInstanced.shader).
		m_ParticleShader.emplace("ParticleInstanced.shader");

		BuildScene();
	}

	~Application() { Shutdown(); }

	void Run()
	{
		double fpsTimer = glfwGetTime();
		int frameCount = 0;

		while (!glfwWindowShouldClose(m_Window))
		{
			TickTime();
			m_ParticleSandbox->Update(m_DeltaTime);
			m_Input.processInput(m_Window, m_Camera, m_DeltaTime);
			m_World.Update(m_DeltaTime * 10.0f);
			Render();
			glfwSwapBuffers(m_Window);
			glfwPollEvents();

			// --- FPS counter (prints once per second) ---
			++frameCount;
			double now = glfwGetTime();
			double elapsed = now - fpsTimer;
			if (elapsed >= 1.0)
			{
				double fps = frameCount / elapsed;
				double msPerFrame = 1000.0 * elapsed / frameCount;
				std::cout << "FPS: " << fps
					<< "  (" << msPerFrame << " ms/frame)"
					<< "  Particles: " << m_ParticleSandbox->GetParticles().size()
					<< std::endl;
				frameCount = 0;
				fpsTimer = now;
			}
		}
	}

private:
	void Init()
	{
		if (!glfwInit())
			throw std::runtime_error("Failed to initialize GLFW");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		m_Window = glfwCreateWindow(m_Config.width, m_Config.height,
			m_Config.title, nullptr, nullptr);
		if (!m_Window)
		{
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwMakeContextCurrent(m_Window);
		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

		glfwSetWindowUserPointer(m_Window, this);
		glfwSetFramebufferSizeCallback(m_Window, OnFramebufferResize);

		m_Input.SetCallbacks(m_Window, m_Camera);

		if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
			throw std::runtime_error("Failed to initialize GLAD");

		glEnable(GL_DEPTH_TEST);

		// Cache projection only rebuilt on resize
		RebuildProjection();
	}

	void BuildScene()
	{
		auto Sun = std::make_shared<SimObject>(
			std::make_unique<Sphere>(2.0f, 64),
			PhysicsObject(glm::vec3(0.0f, 20.0f, 0.0f),
				1.8729e+10f,            // mass ? drives all orbital speeds
				0.0001f,                // radius
				glm::vec3(0.0f),        // no initial velocity
				0.0f,                   // restitution
				true,                   // isStatic
				glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
			Material::Emissive(glm::vec3(1.0f, 0.95f, 0.7f)));

		// Remember where/what the light is so the particle shader (a
		// separate program with its own uniform state) can be lit
		// consistently too - see Render(). A proper Light manager shared by
		// every shader would be a cleaner long-term fix.
		m_LightPosition = glm::vec3(0.0f, 20.0f, 0.0f);
		m_LightColor = glm::vec3(1.0f, 0.95f, 0.7f);

		m_ParticleSandbox = std::make_unique<ParticleSandbox>(100, glm::vec3(0.0f, 10.0f, 0.0f),
			glm::vec3(10.0f, 10.0f, 10.0f));

		m_Scene.Add(Sun);		m_World.AddPhysicsObject(Sun);
	}

	void TickTime()
	{
		constexpr float MAX_DELTA = 1.0f / 120.0f;
		const float now = static_cast<float>(glfwGetTime());
		m_DeltaTime = std::min(now - m_LastFrame, MAX_DELTA);
		m_LastFrame = now;
	}

	void Render()
	{
		m_Renderer.Clear();

		// --- Regular scene (rigid bodies, per-object 'model' uniform) ---
		m_Shader->Bind();
		m_Shader->SetUniform3fv("u_ViewPos", m_Camera.Position);
		m_Shader->SetUniformMat4("projection", m_Projection);
		m_Shader->SetUniformMat4("view", m_Camera.GetViewMatrix());
		m_Scene.Draw(*m_Shader);

		// --- Particles (single instanced draw call) ---
		m_ParticleShader->Bind();
		m_ParticleShader->SetUniform3fv("u_ViewPos", m_Camera.Position);
		m_ParticleShader->SetUniformMat4("projection", m_Projection);
		m_ParticleShader->SetUniformMat4("view", m_Camera.GetViewMatrix());
		m_ParticleShader->SetUniform3fv("u_Light.position", m_LightPosition);
		m_ParticleShader->SetUniform3fv("u_Light.ambient", m_LightColor * 0.2f);
		m_ParticleShader->SetUniform3fv("u_Light.diffuse", m_LightColor * 0.8f);
		m_ParticleShader->SetUniform3fv("u_Light.specular", m_LightColor);
		m_ParticleSandbox->Draw(*m_ParticleShader);
	}

	void RebuildProjection()
	{
		m_Projection = glm::perspective(
			glm::radians(m_Camera.Zoom),
			static_cast<float>(m_Config.width) / static_cast<float>(m_Config.height),
			0.1f, 500.0f);
	}

	static void OnFramebufferResize(GLFWwindow* window, int width, int height)
	{
		auto* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
		app->m_Config.width = static_cast<unsigned int>(width);
		app->m_Config.height = static_cast<unsigned int>(height);
		glViewport(0, 0, width, height);
		app->RebuildProjection();
	}

	void Shutdown()
	{
		glfwDestroyWindow(m_Window);
		glfwTerminate();
	}

	WindowConfig    m_Config;
	GLFWwindow* m_Window = nullptr;

	Camera          m_Camera;
	InputProcesses  m_Input;
	Renderer        m_Renderer;
	std::optional<Shader> m_Shader;
	std::optional<Shader> m_ParticleShader;
	Scene           m_Scene;
	PhysicsWorld    m_World;
	std::unique_ptr<ParticleSandbox> m_ParticleSandbox;

	glm::vec3       m_LightPosition{ 0.0f };
	glm::vec3       m_LightColor{ 1.0f };

	glm::mat4       m_Projection;

	float           m_DeltaTime = 0.0f;
	float           m_LastFrame = 0.0f;
};
