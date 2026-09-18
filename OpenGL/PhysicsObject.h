#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

enum class ColliderType { Sphere, Box };

class PhysicsObject
{
public:
	PhysicsObject(glm::vec3 position, float mass, float radius, glm::vec3 iVelocity,
		float restitution = 0.8f, bool isStatic = false, glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f)); // Sphere

	PhysicsObject(glm::vec3 position, float mass, glm::vec3 halfExtents, glm::vec3 iVelocity,
		float restitution = 0.8f, bool isStatic = false, glm::quat orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f)); // Box

	ColliderType m_ColliderType;
	float        m_Radius = 0.0f;        // Sphere
	glm::vec3    m_HalfExtents{ 0.0f };    // Box

	glm::vec3 m_Position, m_Velocity, m_Acceleration;
	float     m_Mass, m_InvMass, m_Restitution, m_Friction;
	bool      m_IsStatic;
	glm::quat m_Orientation;

	void update(float dt);
	void ApplyForce(const glm::vec3& force);
	void Reset();
	void setPosition(const glm::vec3& p);
	glm::vec3 getPosition() const;
	glm::quat getOrientation() const { return m_Orientation; }
};