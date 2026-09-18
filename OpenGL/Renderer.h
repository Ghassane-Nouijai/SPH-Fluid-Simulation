#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT (GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);

class Renderer
{
public:

    void Draw(const VertexArray& VAO, const IndexBuffer& EBO, const Shader& Shader);
    void Clear() const;
};