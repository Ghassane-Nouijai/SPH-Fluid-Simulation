#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Camera;

class InputProcesses
{
public:
	InputProcesses();
	~InputProcesses();

	void processInput(GLFWwindow* window, Camera& camera, float deltaTime);
	void SetCallbacks(GLFWwindow* window, Camera& camera);

private:
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	Camera* m_Camera = nullptr;
	float m_LastX = 0.0f;
	float m_LastY = 0.0f;
	bool m_FirstMouse = true;
};