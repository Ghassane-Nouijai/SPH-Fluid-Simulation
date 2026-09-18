#include "Renderer.h"

#include <iostream>

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "[OpenGL ERROR] (" << error << ") " << function << " " << file << ": " << line << std::endl;
        return false;
    }
    return true;
}

void Renderer::Draw(const VertexArray& VAO, const IndexBuffer& EBO, const Shader& Shader)
{
	Shader.Bind();
	VAO.Bind();
	EBO.Bind();

	GLCall(glDrawElements(GL_TRIANGLES, EBO.GetCount(), GL_UNSIGNED_INT, nullptr));
}
void Renderer::Clear() const
{
	GLCall(glClearColor(0.1f, 0.1f, 0.1f, 1.0f));
	GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}