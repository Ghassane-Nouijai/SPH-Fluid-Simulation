#pragma once
#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray
{
private:
	unsigned int m_RendererID;
public:
	VertexArray();
	~VertexArray();

	// baseAttribIndex: first attribute location this buffer's elements are
	// bound to. Lets a second (or third...) buffer share the same VAO
	// without clashing attribute locations with a previous AddBuffer() call.
	// attribDivisor: 0 = advance once per vertex (normal per-vertex data),
	//                1 = advance once per instance (for instanced rendering).
	// Both default to their old values, so every existing call site
	// (AddBuffer(vbo, layout)) keeps compiling and behaving exactly as before.
	void AddBuffer(const VertexBuffer& VBO, const VertexBufferLayout& layout,
		unsigned int baseAttribIndex = 0, unsigned int attribDivisor = 0);

	void Bind() const;
	void Unbind() const;
};
