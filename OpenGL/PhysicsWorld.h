#pragma once

#include <vector>
#pragma once
#include "SimObject.h"
#include "CollisionDetection.h"
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();

	void AddPhysicsObject(std::shared_ptr<SimObject> object);
	void RemovePhysicsObject(std::shared_ptr<SimObject> object);
	void Update(float deltaTime);

private:
	std::vector<std::shared_ptr<SimObject>> m_SimObjects;
	glm::vec3 m_Gravity;

	void ResolveCollisions();
	void ResolvePair(PhysicsObject& a, PhysicsObject& b, const CollisionDetection& info);
};