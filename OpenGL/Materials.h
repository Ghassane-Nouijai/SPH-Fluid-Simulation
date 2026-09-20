#pragma once
#include <glm/glm.hpp>

struct Material
{
	glm::vec3 ambient = glm::vec3(0.0f);
	glm::vec3 diffuse = glm::vec3(0.0f);
	glm::vec3 specular = glm::vec3(0.0f);
	glm::vec3 emissive = glm::vec3(0.0f); 
	float     shininess = 1.0f;
	bool      isEmissive = false;

	static Material Gold()
	{
		return { {0.24f, 0.19f, 0.07f},
				 {0.75f, 0.60f, 0.22f},
				 {0.62f, 0.55f, 0.36f},
				 {0.0f,  0.0f,  0.0f },
				  51.2f, false };
	}

	static Material Rubber()
	{
		return { {0.05f, 0.05f, 0.05f},
				 {0.5f,  0.5f,  0.5f },
				 {0.7f,  0.7f,  0.7f },
				 {0.0f,  0.0f,  0.0f },
				  10.0f, false };
	}

	
	
	
	
	
	static Material Water()
	{
		return { {0.10f, 0.20f, 0.30f},
				 {0.20f, 0.55f, 0.85f},
				 {0.90f, 0.90f, 0.95f},
				 {0.0f,  0.0f,  0.0f },
				  90.0f, false };
	}

	static Material Emissive(glm::vec3 color = glm::vec3(1.0f))
	{
		Material m{};
		m.emissive = color;
		m.isEmissive = true;
		return m;
	}
};