#pragma once

#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// Standard SPH smoothing kernels (Muller, Charypar, Gross 2003).
// All kernels are zero outside the smoothing radius 'h'.
// NOTE: the pow(h, n) terms are recomputed every call for clarity.
// Once you have this working, precompute the normalization constants
// per-frame (they only depend on h, not on r) to save a good chunk of CPU time.
namespace SPHKernels
{
	// Used for density estimation: rho_i = sum_j m_j * Poly6(r_ij, h)
	inline float Poly6(float r, float h)
	{
		if (r < 0.0f || r > h)
			return 0.0f;

		float h2 = h * h;
		float diff = h2 - r * r;
		float coeff = 315.0f / (64.0f * glm::pi<float>() * std::pow(h, 9));
		return coeff * diff * diff * diff;
	}

	// Gradient of the Spiky kernel: used for pressure forces.
	// Spiky is preferred over Poly6 for pressure because its gradient does not
	// vanish as r -> 0, which prevents particle clustering.
	// rVec = (posI - posJ), r = length(rVec)
	inline glm::vec3 SpikyGradient(const glm::vec3& rVec, float r, float h)
	{
		if (r <= 0.0f || r > h)
			return glm::vec3(0.0f);

		float coeff = -45.0f / (glm::pi<float>() * std::pow(h, 6));
		float diff = (h - r) * (h - r);
		return coeff * diff * (rVec / r); // rVec / r = unit direction from j to i
	}

	// Laplacian of the Viscosity kernel: used for viscosity forces.
	// Chosen because its Laplacian is positive everywhere in [0, h],
	// which keeps the viscosity force physically stable (always damping).
	inline float ViscosityLaplacian(float r, float h)
	{
		if (r < 0.0f || r > h)
			return 0.0f;

		float coeff = 45.0f / (glm::pi<float>() * std::pow(h, 6));
		return coeff * (h - r);
	}
}