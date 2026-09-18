#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "IRenderable.h"
#include "Mesh.h"
#include "Shader.h"
#include "PhysicsObject.h"

class Sphere : public IRenderable
{
private:
	float m_Radius;
	unsigned int m_Precision; // number of vertices in each circle
	std::unique_ptr<Mesh> m_Mesh;

public:

	Sphere(float radius, unsigned int precision);

	std::vector<float> CreateVertices(float radius, unsigned int precision);
	std::vector<unsigned int> CreateIndices(unsigned int precision);

	void Draw(Shader& shader, const glm::vec3& position, const glm::quat& orientation) override;
};