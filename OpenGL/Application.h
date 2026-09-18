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

		BuildScene();
	}

	~Application() { Shutdown(); }

	void Run()
	{
		while (!glfwWindowShouldClose(m_Window))
		{
			TickTime();
			m_Input.processInput(m_Window, m_Camera, m_DeltaTime);
			m_World.Update(m_DeltaTime * 10.0f);
			Render();
			glfwSwapBuffers(m_Window);
			glfwPollEvents();
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
			PhysicsObject(glm::vec3(0.0f, 0.0f, 0.0f),
				1.8729e+10f,            // mass — drives all orbital speeds
				0.0001f,                   // radius
				glm::vec3(0.0f),        // no initial velocity
				0.0f,                   // restitution
				false,                   // isStatic
				glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
			Material::Emissive(glm::vec3(1.0f, 0.95f, 0.7f)));

		// --- Planet 1  |  r =   5.0  |  T ~   62.8s  |  v = 0.500000 -----------
		auto Planet1 = std::make_shared<SimObject>(
			std::make_unique<Sphere>(0.25f, 32),
			PhysicsObject(glm::vec3(5.0f, 0.0f, 0.0f),
				100000000.0f,
				0.25f,
				glm::vec3(0.0f, 0.0f, 0.500000f),
				0.0f, false,
				glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
			Material::Gold());

		// --- Planet 2  |  r =  10.0  |  T ~  177.7s  |  v = 0.353553 -----------
		auto Planet2 = std::make_shared<SimObject>(
			std::make_unique<Sphere>(0.30f, 32),
			PhysicsObject(glm::vec3(10.0f, 0.0f, 0.0f),
				100000000.0f,
				0.30f,
				glm::vec3(0.0f, 0.0f, 0.353553f),
				0.0f, false,
				glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
			Material::Rubber());

		// --- Planet 3  |  r =  20.0  |  T ~  502.7s  |  v = 0.250000 -----------
		auto Planet3 = std::make_shared<SimObject>(
			std::make_unique<Sphere>(0.22f, 32),
			PhysicsObject(glm::vec3(20.0f, 0.0f, 0.0f),
				10000000000.0f,
				0.22f,
				glm::vec3(0.0f, 0.0f, 0.250000f),
				0.0f, false,
				glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
			Material::Gold());

		// --- Planet 4  |  r =  35.0  |  T ~ 1163.7s  |  v = 0.188982 -----------
		auto Planet4 = std::make_shared<SimObject>(
			std::make_unique<Sphere>(0.35f, 32),
			PhysicsObject(glm::vec3(35.0f, 0.0f, 0.0f),
				200000000.0f,
				0.35f,
				glm::vec3(0.0f, 0.0f, 0.188982f),
				0.0f, false,
				glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
			Material::Rubber());

		// --- Planet 5  |  r =  55.0  |  T ~ 2292.3s  |  v = 0.150756 -----------
		auto Planet5 = std::make_shared<SimObject>(
			std::make_unique<Sphere>(0.88f, 32),
			PhysicsObject(glm::vec3(55.0f, 0.0f, 0.0f),
				800000000.0f,
				0.88f,
				glm::vec3(0.0f, 0.0f, 0.150756f),
				0.0f, false,
				glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
			Material::Gold());

		// --- Planet 6  |  r =  80.0  |  T ~ 4021.2s  |  v = 0.125000 -----------
		auto Planet6 = std::make_shared<SimObject>(
			std::make_unique<Sphere>(0.80f, 32),
			PhysicsObject(glm::vec3(80.0f, 0.0f, 0.0f),
				900000000.0f,
				0.002f,
				glm::vec3(0.0f, 0.0f, 0.4000f),
				0.0f, false,
				glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
			Material::Rubber());

		// --- Planet 7  |  r = 110.0  |  T ~ 6483.6s  |  v = 0.106600 -----------
		auto Planet7 = std::make_shared<SimObject>(
			std::make_unique<Sphere>(1.0f, 32),
			PhysicsObject(glm::vec3(110.0f, 0.0f, 0.0f),
				20000000000.0f,
				0.0f,
				glm::vec3(0.0f, 0.0f, 0.106600f),
				0.0f, false,
				glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
			Material::Gold());

		m_Scene.Add(Sun);       m_World.AddPhysicsObject(Sun);

		m_Scene.Add(Planet1);   m_World.AddPhysicsObject(Planet1);
		m_Scene.Add(Planet2);   m_World.AddPhysicsObject(Planet2);
		m_Scene.Add(Planet3);   m_World.AddPhysicsObject(Planet3);
		m_Scene.Add(Planet4);   m_World.AddPhysicsObject(Planet4);
		m_Scene.Add(Planet5);   m_World.AddPhysicsObject(Planet5);
		m_Scene.Add(Planet6);   m_World.AddPhysicsObject(Planet6);
		m_Scene.Add(Planet7);   m_World.AddPhysicsObject(Planet7);
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
		m_Shader->Bind();

		m_Shader->SetUniform3fv("u_ViewPos", m_Camera.Position);
		m_Shader->SetUniformMat4("projection", m_Projection);          // cached
		m_Shader->SetUniformMat4("view", m_Camera.GetViewMatrix());

		m_Scene.Draw(*m_Shader);
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
	Scene           m_Scene;
	PhysicsWorld    m_World;

	glm::mat4       m_Projection;

	float           m_DeltaTime = 0.0f;
	float           m_LastFrame = 0.0f;
};