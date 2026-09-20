#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <cstdint>

// Uniform spatial hash grid.
// Cell size should equal the SPH smoothing radius 'h'. With that choice,
// every particle within distance h of a query point is guaranteed to be
// found by scanning only the 3x3x3 block of cells around the query point's
// own cell - no need to search the whole domain.
class SpatialHashGrid
{
public:
	void Build(const std::vector<glm::vec3>& positions, float cellSize)
	{
		m_CellSize = cellSize;
		m_Buckets.clear();
		m_Buckets.reserve(positions.size());

		for (uint32_t i = 0; i < positions.size(); ++i)
			m_Buckets[HashCell(CellCoord(positions[i]))].push_back(i);
	}

	// Appends indices of particles that MIGHT be within range of 'position'.
	// The caller still needs to check the exact distance (this only narrows
	// the candidate set down from "all particles" to "nearby particles").
	void Query(const glm::vec3& position, std::vector<uint32_t>& outIndices) const
	{
		glm::ivec3 center = CellCoord(position);

		for (int dx = -1; dx <= 1; ++dx)
			for (int dy = -1; dy <= 1; ++dy)
				for (int dz = -1; dz <= 1; ++dz)
				{
					auto it = m_Buckets.find(HashCell(center + glm::ivec3(dx, dy, dz)));
					if (it != m_Buckets.end())
						outIndices.insert(outIndices.end(), it->second.begin(), it->second.end());
				}
	}

	void Clear()
	{
		m_Buckets.clear();
	}

private:
	glm::ivec3 CellCoord(const glm::vec3& p) const
	{
		return glm::ivec3(glm::floor(p / m_CellSize));
	}

	int64_t HashCell(const glm::ivec3& c) const
	{
		// Large primes reduce collisions; combine into a single 64-bit key.
		constexpr int64_t P1 = 73856093, P2 = 19349663, P3 = 83492791;
		return (static_cast<int64_t>(c.x) * P1) ^
			(static_cast<int64_t>(c.y) * P2) ^
			(static_cast<int64_t>(c.z) * P3);
	}

	float m_CellSize = 1.0f;
	std::unordered_map<int64_t, std::vector<uint32_t>> m_Buckets;
};


