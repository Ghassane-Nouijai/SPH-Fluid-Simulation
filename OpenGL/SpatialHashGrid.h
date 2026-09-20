#pragma once

#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <utility>
#include <cstdint>

class SpatialHashGrid
{
public:
	void Build(const std::vector<glm::vec3>& positions, float cellSize)
	{
		m_CellSize = cellSize;
		const std::size_t n = positions.size();

		m_SortedIndices.resize(n);
		m_Hashes.resize(n);

		for (std::size_t i = 0; i < n; ++i)
		{
			m_Hashes[i] = HashCell(CellCoord(positions[i]));
			m_SortedIndices[i] = static_cast<uint32_t>(i);
		}

		std::sort(m_SortedIndices.begin(), m_SortedIndices.end(),
			[this](uint32_t a, uint32_t b) { return m_Hashes[a] < m_Hashes[b]; });

		m_CellRanges.clear();

		std::size_t rangeStart = 0;
		for (std::size_t k = 0; k < n; ++k)
		{
			int64_t hash = m_Hashes[m_SortedIndices[k]];
			bool lastInCell = (k + 1 == n) || (m_Hashes[m_SortedIndices[k + 1]] != hash);
			if (lastInCell)
			{
				m_CellRanges[hash] = { static_cast<uint32_t>(rangeStart),
										static_cast<uint32_t>(k - rangeStart + 1) };
				rangeStart = k + 1;
			}
		}
	}

	void Query(const glm::vec3& position, std::vector<uint32_t>& outIndices) const
	{
		glm::ivec3 center = CellCoord(position);

		for (int dx = -1; dx <= 1; ++dx)
			for (int dy = -1; dy <= 1; ++dy)
				for (int dz = -1; dz <= 1; ++dz)
				{
					auto it = m_CellRanges.find(HashCell(center + glm::ivec3(dx, dy, dz)));
					if (it == m_CellRanges.end())
						continue;

					uint32_t start = it->second.first;
					uint32_t count = it->second.second;
					outIndices.insert(outIndices.end(),
						m_SortedIndices.begin() + start,
						m_SortedIndices.begin() + start + count);
				}
	}

	void Clear()
	{
		m_SortedIndices.clear();
		m_Hashes.clear();
		m_CellRanges.clear();
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

	std::vector<uint32_t> m_SortedIndices;  // particle indices, sorted by cell hash
	std::vector<int64_t>  m_Hashes;         // m_Hashes[i] = cell hash of particle i (unsorted, indexed by original particle index)

	std::unordered_map<int64_t, std::pair<uint32_t, uint32_t>> m_CellRanges;
};
