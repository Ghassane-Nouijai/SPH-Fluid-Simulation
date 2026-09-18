#pragma once
#include "IRenderable.h"
#include "Shader.h"
#pragma once
#include "SimObject.h"
#include <vector>
#include <memory>

class Scene
{
private:
	std::vector<std::shared_ptr<SimObject>> m_Objects;
public:
	void Add(std::shared_ptr<SimObject> object)
	{
		m_Objects.push_back(object);
	}

	void Draw(Shader& shader)
	{
		for (auto& obj : m_Objects)
			obj->Draw(shader);
	}
};