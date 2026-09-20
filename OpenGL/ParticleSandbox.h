#pragma once

#include <cstddef>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Materials.h"
#include "SpatialHashGrid.h"

class IRenderable;
class Shader;

// Full particle state required by the SPH solver.
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

    // --- SPH tuning parameters (exposed so you can tweak live / from UI) ---
    float m_SmoothingRadius = 0.4f;   // h
    float m_RestDensity = 1000.0f;    // rho0, roughly water-like
    float m_GasConstant = 200.0f;     // k, stiffness of the pressure EOS
    float m_Viscosity = 3.5f;         // mu
    float m_MaxSpeed = 25.0f;         // simple safety clamp against blow-ups

private:
    void CreateParticles(std::size_t particleCount);

    // One-time calibration run right after CreateParticles(): finds the
    // uniform particle mass that makes the *initial* packing evaluate to
    // m_RestDensity. Without this, mass is an arbitrary placeholder and the
    // pressure term never turns on (see comment at the definition).
    void InitializeParticleMasses();

    // SPH pipeline steps, run in this order every Update():
    void BuildNeighborGrid();
    void ComputeDensityPressure();
    void ComputeForces();
    void Integrate(FluidParticle& particle, float deltaTime);
    void ResolveBoxCollision(FluidParticle& particle);
    void SetParticleMaterial(Shader& shader) const;

    std::vector<FluidParticle> m_Particles;
    std::unique_ptr<IRenderable> m_ParticleMesh;

    SpatialHashGrid m_Grid;
    // Scratch buffer reused every frame to avoid reallocating per-particle.
    std::vector<uint32_t> m_NeighborScratch;

    // Scratch buffer reused every frame to upload instanced draw data
    // (4 floats per particle: position.xyz + radius). See Sphere's
    // UpdateInstances()/DrawInstanced() for the consumer side.
    std::vector<float> m_InstanceScratch;

    glm::vec3 m_BoxCenter;
    glm::vec3 m_BoxHalfExtents;
    glm::vec3 m_Gravity;
    Material m_Material;

    float m_Restitution;
    float m_LinearDamping;
    glm::quat m_IdentityOrientation;
};