#include "Mesh.h"

Mesh::Mesh(const std::vector<float>& vertices, const std::vector<unsigned int>& indices, unsigned int floatsPerVertex)
	: m_VBO(vertices, vertices.size() * sizeof(float)),
	m_EBO(indices)
{
	m_VAO.Bind();
	m_VBO.Bind(); // ensure VBO is bound before layout
	VertexBufferLayout layout;
	layout.Push<float>(floatsPerVertex);
	layout.Push<float>(3);
	m_VAO.AddBuffer(m_VBO, layout);
	m_VAO.Unbind();
}

void Mesh::Bind() const
{
	m_VAO.Bind();
	m_EBO.Bind();
}

void Mesh::Unbind() const
{
	m_VAO.Unbind();
	m_EBO.Unbind();
}
