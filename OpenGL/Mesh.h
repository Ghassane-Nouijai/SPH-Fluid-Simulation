#pragma once
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "Shader.h"
#include "Renderer.h"
#include <vector>

class Mesh
{
public:
	Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices, unsigned int floatsPerVertex);
	void Bind() const;
	void Unbind() const;

	unsigned int GetCount() const { return m_EBO.GetCount(); }

private:
	VertexArray  m_VAO;
	VertexBuffer m_VBO;
	IndexBuffer  m_EBO;
};
