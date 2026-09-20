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
 
    void GetBounds(glm::vec3& outMin, glm::vec3& outMax) const;

    void PrintDebugInfo() const;
    
    float m_SmoothingRadius = 0.8f;   
    float m_RestDensity = 100.0f;    
    float m_GasConstant = 200.0f;     
    float m_Viscosity = 1.0f;         
    float m_MaxSpeed = 25.0f;         

    
    float m_RenderScale = 3.0f;

private:
    void CreateParticles(std::size_t particleCount);

    void InitializeParticleMasses();

    void BuildNeighborGrid();
    void ComputeDensityPressure();
    void ComputeForces();
    void Integrate(FluidParticle& particle, float deltaTime);
    void ResolveBoxCollision(FluidParticle& particle);
    void SetParticleMaterial(Shader& shader) const;
    void UpdateKernelConstants();

    std::vector<FluidParticle> m_Particles;
    std::unique_ptr<IRenderable> m_ParticleMesh;

    SpatialHashGrid m_Grid;
    
    std::vector<uint32_t> m_NeighborScratch;
    std::vector<glm::vec3> m_PositionScratch;

    std::vector<float> m_InstanceScratch;

    glm::vec3 m_BoxCenter;
    glm::vec3 m_BoxHalfExtents;
    glm::vec3 m_Gravity;
    Material m_Material;

    float m_Restitution;
    float m_LinearDamping;
    glm::quat m_IdentityOrientation;

    float m_Accumulator = 0.0f;
    float m_KernelRadius = -1.0f;
    float m_Poly6Coefficient = 0.0f;
    float m_SpikyCoefficient = 0.0f;
    float m_ViscosityCoefficient = 0.0f;
};
