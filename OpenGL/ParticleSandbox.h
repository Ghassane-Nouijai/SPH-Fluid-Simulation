#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Materials.h"

class IRenderable;
class Shader;

// State intentionally includes the fields required by a future SPH pass.
// Density, pressure and force are not yet used to simulate fluid behaviour.
struct FluidParticle
{
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 force;

    float density;
    float pressure;
    float mass;
    float radius;
};

class ParticleSandbox
{
public:
    ParticleSandbox(std::size_t particleCount,
        const glm::vec3& boxCenter,
        const glm::vec3& boxHalfExtents);

    void Update(float deltaTime);
    void Draw(Shader& shader);

    const std::vector<FluidParticle>& GetParticles() const;

private:
    void CreateParticles(std::size_t particleCount);
    void Integrate(FluidParticle& particle, float deltaTime);
    void ResolveBoxCollision(FluidParticle& particle);
    void SetParticleMaterial(Shader& shader) const;

    std::vector<FluidParticle> m_Particles;
    std::unique_ptr<IRenderable> m_ParticleMesh;

    glm::vec3 m_BoxCenter;
    glm::vec3 m_BoxHalfExtents;
    glm::vec3 m_Gravity;
    Material m_Material;

    float m_Restitution;
    float m_LinearDamping;
    glm::quat m_IdentityOrientation;
};