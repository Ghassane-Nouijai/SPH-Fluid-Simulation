#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include <memory>
#include <vector>

#include "IRenderable.h"
#include "Mesh.h"
#include "Shader.h"

class Rectangle : public IRenderable
{
private:
	float m_SideA;
	float m_SideB;
	unsigned int m_Precision; // number of vertices in each circle
	std::unique_ptr<Mesh> m_Mesh;

public:

	Rectangle(float sideA, float sideB);

	std::vector<float> CreateVertices(float sideA, float sideB);

	void Draw(Shader& shader, const glm::vec3& position, const glm::quat& orientation) override;
};