#include "Rectangle.h"

Rectangle::Rectangle(float sideA, float sideB) : m_SideA(sideA), m_SideB(sideB)
{
	std::vector<float>        vertices = CreateVertices(m_SideA, m_SideB);
	std::vector<unsigned int> indices = {
		0, 1, 2,
		2, 3, 0
	};

	m_Mesh = std::make_unique<Mesh>(vertices, indices, 3);
}

std::vector<float> Rectangle::CreateVertices(float sideA, float sideB)
{
	std::vector<float> vertices = {
		-sideA / 2.0f, -sideB / 2.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Bottom-left
		 sideA / 2.0f, -sideB / 2.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Bottom-right
		 sideA / 2.0f,  sideB / 2.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Top-right
		-sideA / 2.0f,  sideB / 2.0f, 0.0f, 0.0f, 0.0f, 1.0f  // Top-left
	};
	return vertices;
}

void Rectangle::Draw(Shader& shader, const glm::vec3& position, const glm::quat& orientation)
{
	m_Mesh->Bind();

	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotation = glm::mat4_cast(orientation);
	glm::mat4 model = translation * rotation;

	shader.SetUniformMat4("model", model);
	GLCall(glDrawElements(GL_TRIANGLES, m_Mesh->GetCount(), GL_UNSIGNED_INT, nullptr));

	m_Mesh->Unbind();
}