#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>






namespace SPHKernels
{
	
	inline float Poly6(float r, float h)
	{
		if (r < 0.0f || r > h)
			return 0.0f;

		float h2 = h * h;
		float diff = h2 - r * r;
		float coeff = 315.0f / (64.0f * glm::pi<float>() * std::pow(h, 9));
		return coeff * diff * diff * diff;
	}

	inline float Poly6(float r, float h, float coefficient)
	{
		if (r < 0.0f || r > h)
			return 0.0f;

		float h2 = h * h;
		float diff = h2 - r * r;
		return coefficient * diff * diff * diff;
	}

	
	
	
	
	inline glm::vec3 SpikyGradient(const glm::vec3& rVec, float r, float h)
	{
		if (r <= 0.0f || r > h)
			return glm::vec3(0.0f);

		float coeff = -45.0f / (glm::pi<float>() * std::pow(h, 6));
		float diff = (h - r) * (h - r);
		return coeff * diff * (rVec / r); 
	}

	inline glm::vec3 SpikyGradient(const glm::vec3& rVec, float r, float h, float coefficient)
	{
		if (r <= 0.0f || r > h)
			return glm::vec3(0.0f);

		float diff = (h - r) * (h - r);
		return coefficient * diff * (rVec / r);
	}

	
	
	
	inline float ViscosityLaplacian(float r, float h)
	{
		if (r < 0.0f || r > h)
			return 0.0f;

		float coeff = 45.0f / (glm::pi<float>() * std::pow(h, 6));
		return coeff * (h - r);
	}

	inline float ViscosityLaplacian(float r, float h, float coefficient)
	{
		if (r < 0.0f || r > h)
			return 0.0f;

		return coefficient * (h - r);
	}
}