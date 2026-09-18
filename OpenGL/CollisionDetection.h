#pragma once

#include <glm/glm.hpp>

struct CollisionDetection
{
	bool m_CollisionDetected;
	glm::vec3 m_Normal;
	float m_Depth;
};

inline CollisionDetection SphereSphereCollision(const glm::vec3& posA, float radiusA, 
	const glm::vec3& posB, float radiusB)
{
	float distance = glm::length(posA - posB);
	if (distance < (radiusA + radiusB))
	{
		float depth = (radiusA + radiusB) - distance;
		glm::vec3 normal = glm::normalize(posB - posA);
		return { true, normal, depth };
	}
	return { false, glm::vec3(0.0f), 0.0f };
}

inline CollisionDetection SpherePlaneCollision(const glm::vec3& spherePos, float sphereRadius, 
	const glm::vec3& planeNormal, float planeDistance)
{
	float distance = glm::dot(spherePos, planeNormal) - planeDistance;
	if (distance < sphereRadius)
	{
		float depth = sphereRadius - distance;
		glm::vec3 normal = glm::normalize(planeNormal);
		return { true, normal, depth };
	}
	return { false, glm::vec3(0.0f), 0.0f };
}

inline CollisionDetection SphereAABBCollision(const glm::vec3& spherePos, float radius,
	const glm::vec3& boxPos, const glm::vec3& halfExtents)
{
	glm::vec3 localPos = spherePos - boxPos;
	glm::vec3 closest = glm::clamp(localPos, -halfExtents, halfExtents);
	glm::vec3 diff = localPos - closest; 

	float distSq = glm::dot(diff, diff);

	if (distSq > 0.00001f)
	{
		if (distSq <= radius * radius)
		{
			float dist = std::sqrt(distSq);
			glm::vec3 normal = -diff / dist;
			float depth = radius - dist;
			return { true, normal, depth };
		}
		return { false, glm::vec3(0.0f), 0.0f };
	}

	// Sphere center is inside the box
	glm::vec3 distToMax = halfExtents - localPos;
	glm::vec3 distToMin = localPos + halfExtents;

	float minDist = distToMax.x;
	glm::vec3 exitDir(1.0f, 0.0f, 0.0f);  // direction OUT of the box

	if (distToMin.x < minDist) { minDist = distToMin.x; exitDir = glm::vec3(-1.0f, 0.0f, 0.0f); }
	if (distToMax.y < minDist) { minDist = distToMax.y; exitDir = glm::vec3(0.0f, 1.0f, 0.0f); }
	if (distToMin.y < minDist) { minDist = distToMin.y; exitDir = glm::vec3(0.0f, -1.0f, 0.0f); }
	if (distToMax.z < minDist) { minDist = distToMax.z; exitDir = glm::vec3(0.0f, 0.0f, 1.0f); }
	if (distToMin.z < minDist) { minDist = distToMin.z; exitDir = glm::vec3(0.0f, 0.0f, -1.0f); }

	return { true, -exitDir, minDist + radius };
}