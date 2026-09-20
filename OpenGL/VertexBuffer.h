#pragma once

#include <vector>

class VertexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_Capacity; // bytes currently allocated on the GPU

public:
	// Static, upload-once buffer. Unchanged from before.
	VertexBuffer(const std::vector<float>& data, unsigned int size);

	// Empty buffer pre-allocated up front for later per-frame updates via
	// SetData(). Pass GL_STREAM_DRAW for data that changes every frame
	// (e.g. per-instance particle transforms) or GL_DYNAMIC_DRAW for data
	// that changes occasionally.
	VertexBuffer(unsigned int sizeBytes, unsigned int usage);

	~VertexBuffer();

	void Bind() const;
	void Unbind() const;

	// Re-uploads 'size' bytes starting at the beginning of the buffer via
	// glBufferSubData (no reallocation) as long as 'size' fits within the
	// capacity the buffer was created with. If it doesn't fit, the buffer is
	// grown with glBufferData - safe, but avoid relying on this path every
	// frame since it forces a reallocation.
	void SetData(const void* data, unsigned int size);
};