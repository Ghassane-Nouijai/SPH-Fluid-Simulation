#include "InstancedMesh.h"
#include "Renderer.h"

#include <iostream>

namespace
{
	unsigned int FloatsPerInstance(const VertexBufferLayout& layout)
	{
		return layout.GetStride() / static_cast<unsigned int>(sizeof(float));
	}
}

InstancedMesh::InstancedMesh(const std::vector<float>& vertices,
	const std::vector<unsigned int>& indices,
	const VertexBufferLayout& vertexLayout,
	const VertexBufferLayout& instanceLayout,
	std::size_t maxInstances)
	: m_VBO(vertices, static_cast<unsigned int>(vertices.size() * sizeof(float)))
	, m_EBO(indices)
	, m_InstanceVBO(static_cast<unsigned int>(maxInstances* instanceLayout.GetStride()), GL_STREAM_DRAW)
	, m_InstanceFloatStride(FloatsPerInstance(instanceLayout))
{
	m_VAO.Bind();

	
	m_VAO.AddBuffer(m_VBO, vertexLayout, 0, 0);

	
	
	unsigned int instanceBaseIndex = static_cast<unsigned int>(vertexLayout.GetElements().size());
	m_VAO.AddBuffer(m_InstanceVBO, instanceLayout, instanceBaseIndex, 1);

	
	
	m_EBO.Bind();

	m_VAO.Unbind();
}

void InstancedMesh::UpdateInstanceData(const std::vector<float>& instanceData)
{
	m_InstanceVBO.SetData(instanceData.data(), static_cast<unsigned int>(instanceData.size() * sizeof(float)));
	m_ActiveInstanceCount = m_InstanceFloatStride
		? static_cast<unsigned int>(instanceData.size() / m_InstanceFloatStride)
		: 0;
}

void InstancedMesh::Draw(unsigned int indexCount) const
{
	if (m_ActiveInstanceCount == 0)
	{
		
		
		
		
		static bool warned = false;
		if (!warned)
		{
			std::cout << "[InstancedMesh] Draw() skipped: 0 active instances - "
				"did you call UpdateInstanceData() with a non-empty buffer this frame?"
				<< std::endl;
			warned = true;
		}
		return;
	}

	m_VAO.Bind();
	GLCall(glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, m_ActiveInstanceCount));
	m_VAO.Unbind();
}