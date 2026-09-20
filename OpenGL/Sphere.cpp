#define _USE_MATH_DEFINES 
#include <cmath>

#include <vector>

#include <glm/glm.hpp>

#include "Renderer.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "VertexBufferLayout.h"
#include "Sphere.h"

float pi = M_PI;

Sphere::Sphere(float radius, unsigned int precision)
	: m_Radius(radius), m_Precision(precision)
{
	std::vector<float>        vertices = CreateVertices(radius, precision);
	std::vector<unsigned int> indices = CreateIndices(precision);

	m_Mesh = std::make_unique<Mesh>(vertices, indices, 3);
}

std::vector<float> Sphere::CreateVertices(float radious, unsigned int precision)
{
	std::vector<float> vertices;
	unsigned int stacks = precision / 2;   // latitude bands
	unsigned int sectors = precision;      // longitude segments

	for (unsigned int i = 0; i <= stacks; i++)
	{
		float phi = pi * (float)i / (float)stacks;
		for (unsigned int j = 0; j < sectors; j++)
		{
			float theta = 2.0f * pi * (float)j / (float)sectors;
			vertices.push_back(radious * sin(phi) * cos(theta));  // x
			vertices.push_back(radious * cos(phi));               // y
			vertices.push_back(radious * sin(phi) * sin(theta));  // z
			vertices.push_back(sin(phi) * cos(theta));  // normal x
			vertices.push_back(cos(phi));               // normal y
			vertices.push_back(sin(phi) * sin(theta));  // normal z
		}
	}
	return vertices;
}

std::vector<unsigned int> Sphere::CreateIndices(unsigned int precision)
{
	std::vector<unsigned int> indices;
	unsigned int stacks = precision / 2;
	unsigned int sectors = precision;

	for (unsigned int row = 0; row < stacks; row++)
	{
		for (unsigned int col = 0; col < sectors; col++)
		{
			unsigned int nextCol = (col + 1) % sectors;

			unsigned int current = row * sectors + col;
			unsigned int currentNext = row * sectors + nextCol;
			unsigned int below = (row + 1) * sectors + col;
			unsigned int belowNext = (row + 1) * sectors + nextCol;

			indices.push_back(current);
			indices.push_back(below);
			indices.push_back(currentNext);

			indices.push_back(currentNext);
			indices.push_back(below);
			indices.push_back(belowNext);
		}
	}
	return indices;
}

void Sphere::Draw(Shader& shader, const glm::vec3& position, const glm::quat& orientation)
{
	m_Mesh->Bind();

	glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 rotation = glm::mat4_cast(orientation);
	glm::mat4 model = translation * rotation;

	shader.SetUniformMat4("model", model);
	GLCall(glDrawElements(GL_TRIANGLES, m_Mesh->GetCount(), GL_UNSIGNED_INT, nullptr));

	m_Mesh->Unbind();
}

void Sphere::BuildInstancedMesh(std::size_t maxInstances)
{
	VertexBufferLayout vertexLayout;
	vertexLayout.Push<float>(3); // position
	vertexLayout.Push<float>(3); // normal

	VertexBufferLayout instanceLayout;
	instanceLayout.Push<float>(4);
	instanceLayout.Push<float>(1);

	std::vector<float> vertices = CreateVertices(m_Radius, m_Precision);
	std::vector<unsigned int> indices = CreateIndices(m_Precision);
	m_IndexCount = static_cast<unsigned int>(indices.size());

	m_InstancedMesh = std::make_unique<InstancedMesh>(
		vertices, indices, vertexLayout, instanceLayout, maxInstances);
	m_MaxInstances = maxInstances;
}

void Sphere::UpdateInstances(const std::vector<float>& instanceData)
{
	constexpr unsigned int kFloatsPerInstance = 5; // pos.xyz, scale, speed
	std::size_t instanceCount = instanceData.size() / kFloatsPerInstance;

	if (!m_InstancedMesh || instanceCount > m_MaxInstances)
		BuildInstancedMesh(instanceCount);

	m_InstancedMesh->UpdateInstanceData(instanceData);
}

void Sphere::DrawInstanced(Shader& shader)
{
	(void)shader; // material/light uniforms are expected to already be set
	if (!m_InstancedMesh)
		return;

	m_InstancedMesh->Draw(m_IndexCount);
}
