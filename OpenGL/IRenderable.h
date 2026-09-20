#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "Shader.h"

class IRenderable {
public:
	virtual void Draw(Shader& shader, const glm::vec3& position, const glm::quat& orientation) = 0;

	// --- Optional GPU-instanced draw path -------------------------------
	// Default implementations are no-ops, so any IRenderable that never
	// needs instancing (Rectangle, one-off meshes, ...) requires no changes
	// at all. Renderables that want it (Sphere) override both.
	//
	// instanceData is a flat, tightly-packed float buffer; how many floats
	// belong to one instance and what they mean (position only? position +
	// scale? a full mat4?) is entirely up to the concrete renderable - it's
	// the same contract UpdateInstances()/DrawInstanced() must agree on
	// internally. See Sphere for a concrete example (4 floats/instance:
	// worldPos.xyz + uniform scale).
	virtual void UpdateInstances(const std::vector<float>& instanceData) {}
	virtual void DrawInstanced(Shader& shader) {}

	virtual ~IRenderable() = default;
};