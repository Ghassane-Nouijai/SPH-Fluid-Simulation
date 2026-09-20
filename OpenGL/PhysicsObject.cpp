#include "PhysicsObject.h"

PhysicsObject::PhysicsObject(glm::vec3 position, float mass, float radius, glm::vec3 iVelocity,
	float restitution, bool isStatic, glm::quat orientation)
	: m_Position(position), m_Velocity(iVelocity), m_Acceleration(0.0f),
	m_Mass(mass), m_Restitution(restitution), m_Radius(radius),
	m_IsStatic(isStatic), m_InvMass(isStatic ? 0.0f : (mass > 0.0f ? 1.0f / mass : 0.0f)),
	m_Friction(0.0f), m_ColliderType(ColliderType::Sphere), m_Orientation(orientation)
{
}

PhysicsObject::PhysicsObject(glm::vec3 position, float mass, glm::vec3 halfExtents, glm::vec3 iVelocity,
	float restitution, bool isStatic, glm::quat orientation)
	: m_Position(position), m_Velocity(iVelocity), m_Acceleration(0.0f),
	m_Mass(mass), m_Restitution(restitution), m_HalfExtents(halfExtents),
	m_IsStatic(isStatic), m_InvMass(isStatic ? 0.0f : (mass > 0.0f ? 1.0f / mass : 0.0f)),
	m_Friction(0.0f), m_ColliderType(ColliderType::Box), m_Orientation(orientation)
{
}

void PhysicsObject::update(float deltaTime)
{
	
	if (this->m_IsStatic)
	{
		return;
	}

	this->m_Velocity += this->m_Acceleration * deltaTime;
	this->m_Position += this->m_Velocity * deltaTime;
}

void PhysicsObject::ApplyForce(const glm::vec3& force)
{
	this->m_Acceleration += force * this->m_InvMass;
}

void PhysicsObject::Reset()
{
	m_Acceleration = glm::vec3(0.0f);
}

void PhysicsObject::setPosition(const glm::vec3& position)
{
	this->m_Position = position;
}

glm::vec3 PhysicsObject::getPosition() const
{
	return this->m_Position;
}