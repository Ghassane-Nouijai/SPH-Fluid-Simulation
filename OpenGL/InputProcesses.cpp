#include "InputProcesses.h"
#include "Camera.h"

InputProcesses::InputProcesses() 
{

}
InputProcesses::~InputProcesses() 
{

}

void InputProcesses::processInput(GLFWwindow* window, Camera& camera, float deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(FORWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(LEFT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(UP, deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
	{
		camera.ProcessKeyboard(DOWN, deltaTime);
	}
}

void InputProcesses::SetCallbacks(GLFWwindow* window, Camera& camera)
{
	m_Camera = &camera;

	// Stash 'this' on the window so the static callbacks below can get back to instance state
	glfwSetWindowUserPointer(window, this);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
}

void InputProcesses::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void InputProcesses::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	InputProcesses* self = static_cast<InputProcesses*>(glfwGetWindowUserPointer(window));
	if (!self || !self->m_Camera) return;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (self->m_FirstMouse)
	{
		self->m_LastX = xpos;
		self->m_LastY = ypos;
		self->m_FirstMouse = false;
	}

	float xoffset = xpos - self->m_LastX;
	float yoffset = self->m_LastY - ypos;

	self->m_LastX = xpos;
	self->m_LastY = ypos;

	self->m_Camera->ProcessMouseMovement(xoffset, yoffset);
}

void InputProcesses::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	InputProcesses* self = static_cast<InputProcesses*>(glfwGetWindowUserPointer(window));
	if (!self || !self->m_Camera) return;

	self->m_Camera->ProcessMouseScroll(static_cast<float>(yoffset));
}