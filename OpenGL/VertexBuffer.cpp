#include "VertexBuffer.h"

#include "Renderer.h"

VertexBuffer::VertexBuffer(const std::vector<float>& data, unsigned int size)
	: m_Capacity(size)
{
	GLCall(glGenBuffers(1, &m_RendererID));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
	GLCall(glBufferData(GL_ARRAY_BUFFER, size, data.data(), GL_STATIC_DRAW));
}

VertexBuffer::VertexBuffer(unsigned int sizeBytes, unsigned int usage)
	: m_Capacity(sizeBytes)
{
	GLCall(glGenBuffers(1, &m_RendererID));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
	GLCall(glBufferData(GL_ARRAY_BUFFER, sizeBytes, nullptr, usage));
}

VertexBuffer::~VertexBuffer()
{
	GLCall(glDeleteBuffers(1, &m_RendererID));
}

void VertexBuffer::Bind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::Unbind() const
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void VertexBuffer::SetData(const void* data, unsigned int size)
{
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
	if (size > m_Capacity)
	{
		// Shouldn't normally trigger if the buffer was sized correctly up
		// front, but grow safely instead of overflowing.
		GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STREAM_DRAW));
		m_Capacity = size;
	}
	else
	{
		GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, size, data));
	}
}
