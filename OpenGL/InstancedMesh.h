#pragma once

#include <cstddef>
#include <vector>

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"

// Generic, reusable building block for GPU instanced rendering.
//
// Any IRenderable that wants an instanced draw path can build one of these
// once (typically lazily, on first use) from the same vertex/index arrays it
// already generates for its normal, single-object mesh - then just call
// UpdateInstanceData() + Draw() every frame instead of one draw call per
// object.
//
// Per-instance layout is entirely up to the caller (position only,
// position+scale, color, a full mat4...) - describe it once via
// 'instanceLayout' and upload matching floats every frame via
// UpdateInstanceData(). This class does not know or care what the floats
// mean; that contract lives between the caller and its own vertex shader.
class InstancedMesh
{
public:
	InstancedMesh(const std::vector<float>& vertices,
		const std::vector<unsigned int>& indices,
		const VertexBufferLayout& vertexLayout,
		const VertexBufferLayout& instanceLayout,
		std::size_t maxInstances);

	// Uploads this frame's per-instance data (tightly packed floats,
	// interpreted according to 'instanceLayout' passed to the constructor).
	// The active instance count for Draw() is derived from data.size().
	void UpdateInstanceData(const std::vector<float>& instanceData);

	void Draw(unsigned int indexCount) const;

private:
	VertexArray  m_VAO;
	VertexBuffer m_VBO;
	IndexBuffer  m_EBO;
	VertexBuffer m_InstanceVBO;

	unsigned int m_InstanceFloatStride; // floats per instance, derived from instanceLayout
	unsigned int m_ActiveInstanceCount = 0;
};