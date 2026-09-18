#include "PhysicsWorld.h"

PhysicsWorld::PhysicsWorld() : m_Gravity(0.0f, -9.81f, 0.0f) {}
PhysicsWorld::~PhysicsWorld() {}

void PhysicsWorld::AddPhysicsObject(std::shared_ptr<SimObject> object)
{
	m_SimObjects.push_back(object);
}

void PhysicsWorld::Update(float deltaTime)
{
	for (auto& obj : m_SimObjects)
	{
		PhysicsObject& phys = obj->GetPhysicsObject();
		if (!phys.m_IsStatic)
		{
			glm::vec3 TotalAcceleration(0.0f);
			const float G = 6.67430e-11f;

			for (const auto& obj2 : m_SimObjects)
			{
				PhysicsObject& phys2 = obj2->GetPhysicsObject();

				if (&phys2 == &phys)
					continue;

				glm::vec3 direction = phys2.getPosition() - phys.getPosition();
				float distSqr = glm::dot(direction, direction);

				if (distSqr < 0.0001f)
					continue;

				float dist = glm::sqrt(distSqr);
				glm::vec3 unitDirection = direction / dist;

				float AccelMagnitude = G * phys2.m_Mass / distSqr;
				TotalAcceleration += unitDirection * AccelMagnitude;
			}

			phys.ApplyForce(TotalAcceleration * phys.m_Mass);
		}
	}

	for (auto& obj : m_SimObjects)
	{
		PhysicsObject& phys = obj->GetPhysicsObject();
		if (!phys.m_IsStatic)
			phys.update(deltaTime);
	}

	ResolveCollisions();

	for (auto& obj : m_SimObjects)
		obj->GetPhysicsObject().Reset();
}

void PhysicsWorld::ResolveCollisions()
{
	for (size_t i = 0; i < m_SimObjects.size(); i++)
		for (size_t j = i + 1; j < m_SimObjects.size(); j++)
		{
			PhysicsObject& a = m_SimObjects[i]->GetPhysicsObject();
			PhysicsObject& b = m_SimObjects[j]->GetPhysicsObject();

			CollisionDetection info{ false, glm::vec3(0.0f), 0.0f };

			if (a.m_ColliderType == ColliderType::Sphere && b.m_ColliderType == ColliderType::Sphere)
			{
				info = SphereSphereCollision(a.getPosition(), a.m_Radius, b.getPosition(), b.m_Radius);
			}
			else if (a.m_ColliderType == ColliderType::Sphere && b.m_ColliderType == ColliderType::Box)
			{
				info = SphereAABBCollision(a.getPosition(), a.m_Radius, b.getPosition(), b.m_HalfExtents);
			}
			else if (a.m_ColliderType == ColliderType::Box && b.m_ColliderType == ColliderType::Sphere)
			{
				info = SphereAABBCollision(b.getPosition(), b.m_Radius, a.getPosition(), a.m_HalfExtents);
				info.m_Normal = -info.m_Normal; 
			}

			if (info.m_CollisionDetected)
				ResolvePair(a, b, info);
		}
}

void PhysicsWorld::ResolvePair(PhysicsObject& a, PhysicsObject& b, const CollisionDetection& info)
{
	float totalInvMass = a.m_InvMass + b.m_InvMass;
	if (totalInvMass <= 0.0f) return;

	// Positional correction
	glm::vec3 correction = info.m_Normal * (info.m_Depth / totalInvMass);
	if (!a.m_IsStatic) a.setPosition(a.getPosition() - correction * a.m_InvMass);
	if (!b.m_IsStatic) b.setPosition(b.getPosition() + correction * b.m_InvMass);

	// Velocity response
	glm::vec3 relVel = b.m_Velocity - a.m_Velocity;
	float     velAlongNormal = glm::dot(relVel, info.m_Normal);
	if (velAlongNormal > 0.0f) return;

	float     restitution = std::min(a.m_Restitution, b.m_Restitution);
	float     impulseMag = -(1.0f + restitution) * velAlongNormal / totalInvMass;
	glm::vec3 impulse = impulseMag * info.m_Normal;

	if (!a.m_IsStatic) a.m_Velocity -= impulse * a.m_InvMass;
	if (!b.m_IsStatic) b.m_Velocity += impulse * b.m_InvMass;
}
