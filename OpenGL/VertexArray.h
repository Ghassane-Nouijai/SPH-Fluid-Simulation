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

	
	
	
	
	
	
	
	void AddBuffer(const VertexBuffer& VBO, const VertexBufferLayout& layout,
		unsigned int baseAttribIndex = 0, unsigned int attribDivisor = 0);

	void Bind() const;
	void Unbind() const;
};
