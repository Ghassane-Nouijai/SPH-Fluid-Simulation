#include "VertexArray.h"
#include "Renderer.h"
#include "VertexBufferLayout.h"

VertexArray::VertexArray()
{
	GLCall(glGenVertexArrays(1, &m_RendererID));
}

VertexArray::~VertexArray()
{
	GLCall(glDeleteVertexArrays(1, &m_RendererID));
}

void VertexArray::AddBuffer(const VertexBuffer& VBO, const VertexBufferLayout& layout,
	unsigned int baseAttribIndex, unsigned int attribDivisor)
{
	Bind();
	VBO.Bind();
	const auto& elements = layout.GetElements();
	unsigned int offset = 0;
	for (unsigned int i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		unsigned int location = baseAttribIndex + i;

		GLCall(glEnableVertexAttribArray(location));
		GLCall(glVertexAttribPointer(location, element.count, element.type, element.normalized, layout.GetStride(), (const void*)offset));

		if (attribDivisor > 0)
			GLCall(glVertexAttribDivisor(location, attribDivisor));

		offset += element.count * VertexBufferElement::GetSizeOftype(element.type);
	}
}

void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(m_RendererID));
}

void VertexArray::Unbind() const
{
	GLCall(glBindVertexArray(0));
}
