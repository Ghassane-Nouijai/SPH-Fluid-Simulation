#pragma once

#include <cstddef>
#include <vector>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"














class InstancedMesh
{
public:
	InstancedMesh(const std::vector<float>& vertices,
		const std::vector<unsigned int>& indices,
		const VertexBufferLayout& vertexLayout,
		const VertexBufferLayout& instanceLayout,
		std::size_t maxInstances);

	
	
	
	void UpdateInstanceData(const std::vector<float>& instanceData);

	void Draw(unsigned int indexCount) const;

private:
	VertexArray  m_VAO;
	VertexBuffer m_VBO;
	IndexBuffer  m_EBO;
	VertexBuffer m_InstanceVBO;

	unsigned int m_InstanceFloatStride; 
	unsigned int m_ActiveInstanceCount = 0;
};