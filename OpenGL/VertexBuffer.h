#pragma once

#include <vector>

class VertexBuffer
{
private:
	unsigned int m_RendererID;
	unsigned int m_Capacity; 

public:
	
	VertexBuffer(const std::vector<float>& data, unsigned int size);

	
	
	
	
	VertexBuffer(unsigned int sizeBytes, unsigned int usage);

	~VertexBuffer();

	void Bind() const;
	void Unbind() const;

	
	
	
	
	
	void SetData(const void* data, unsigned int size);
};