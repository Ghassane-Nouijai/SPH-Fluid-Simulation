#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Shader.h"

class IRenderable {
public:
	virtual void Draw(Shader& shader, const glm::vec3& position, const glm::quat& orientation) = 0;

	
	
	
	
	
	
	
	
	
	
	
	virtual void UpdateInstances(const std::vector<float>& instanceData) {}
	virtual void DrawInstanced(Shader& shader) {}

	virtual ~IRenderable() = default;
};