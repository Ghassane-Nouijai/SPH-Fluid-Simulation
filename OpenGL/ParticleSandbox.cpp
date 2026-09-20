#include "ParticleSandbox.h"

#include <random>
#include <algorithm>
#include <iostream>

#include "IRenderable.h"
#include "Sphere.h"
#include "Shader.h"
#include "SPHKernels.h"

ParticleSandbox::ParticleSandbox(std::size_t particleCount,
    const glm::vec3& boxCenter,
    const glm::vec3& boxHalfExtents)
    : m_BoxCenter(boxCenter)
    , m_BoxHalfExtents(boxHalfExtents)
    , m_Gravity(0.0f, -9.81f, 0.0f)
    , m_Material(Material::Water())
    , m_Restitution(0.3f)
    , m_LinearDamping(0.995f)
    , m_IdentityOrientation(1.0f, 0.0f, 0.0f, 0.0f)
{
    m_ParticleMesh = std::make_unique<Sphere>(1.0f, 6);

    CreateParticles(particleCount);
    InitializeParticleMasses();

    m_NeighborScratch.reserve(128);
    m_PositionScratch.reserve(particleCount);
    m_InstanceScratch.reserve(particleCount * 4);
}

void ParticleSandbox::CreateParticles(std::size_t particleCount)
{
    m_Particles.clear();
    m_Particles.reserve(particleCount);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> jitter(-0.02f, 0.02f);

    
    
    
    float spacing = m_SmoothingRadius * 0.5f;
    int perAxis = static_cast<int>(std::ceil(std::cbrt(static_cast<double>(particleCount))));

    glm::vec3 origin = m_BoxCenter - glm::vec3(perAxis * spacing * 0.5f);

    int created = 0;
    for (int x = 0; x < perAxis && created < static_cast<int>(particleCount); ++x)
        for (int y = 0; y < perAxis && created < static_cast<int>(particleCount); ++y)
            for (int z = 0; z < perAxis && created < static_cast<int>(particleCount); ++z)
            {
                FluidParticle p{};
                p.position = origin + glm::vec3(x, y, z) * spacing
                    + glm::vec3(jitter(rng), jitter(rng), jitter(rng));
                p.velocity = glm::vec3(0.0f);
                p.force = glm::vec3(0.0f);
                p.density = m_RestDensity;
                p.pressure = 0.0f;
                p.mass = 1.0f; 
                p.radius = 0.08f;

                m_Particles.push_back(p);
                ++created;
            }
}

void ParticleSandbox::InitializeParticleMasses()
{
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    
    std::vector<glm::vec3> positions;
    positions.reserve(m_Particles.size());
    for (auto& p : m_Particles)
        positions.push_back(p.position);

    SpatialHashGrid calibrationGrid;
    calibrationGrid.Build(positions, m_SmoothingRadius);

    std::vector<uint32_t> neighbors;
    neighbors.reserve(128);

    float totalDensity = 0.0f;
    for (auto& pi : m_Particles)
    {
        neighbors.clear();
        calibrationGrid.Query(pi.position, neighbors);

        float density = 0.0f;
        for (uint32_t j : neighbors)
        {
            float r = glm::length(pi.position - m_Particles[j].position);
            density += 1.0f * SPHKernels::Poly6(r, m_SmoothingRadius); 
        }
        totalDensity += density;
    }

    float avgDensityWithUnitMass = totalDensity / static_cast<float>(m_Particles.size());
    float calibratedMass = m_RestDensity / std::max(avgDensityWithUnitMass, 1e-6f);

    for (auto& p : m_Particles)
        p.mass = calibratedMass;
}

void ParticleSandbox::BuildNeighborGrid()
{
    m_PositionScratch.clear();
    for (auto& p : m_Particles)
        m_PositionScratch.push_back(p.position);

    m_Grid.Build(m_PositionScratch, m_SmoothingRadius);
}

void ParticleSandbox::ComputeDensityPressure()
{
    for (auto& pi : m_Particles)
    {
        m_NeighborScratch.clear();
        m_Grid.Query(pi.position, m_NeighborScratch);

        float density = 0.0f;
        for (uint32_t j : m_NeighborScratch)
        {
            const FluidParticle& pj = m_Particles[j];
            float r = glm::length(pi.position - pj.position);
            density += pj.mass * SPHKernels::Poly6(r, m_SmoothingRadius, m_Poly6Coefficient);
        }

        pi.density = std::max(density, 1e-6f);
        
        
        pi.pressure = std::max(m_GasConstant * (pi.density - m_RestDensity), 0.0f);
    }
}

void ParticleSandbox::ComputeForces()
{
    for (std::size_t i = 0; i < m_Particles.size(); ++i)
    {
        FluidParticle& pi = m_Particles[i];

        m_NeighborScratch.clear();
        m_Grid.Query(pi.position, m_NeighborScratch);

        glm::vec3 pressureForce(0.0f);
        glm::vec3 viscosityForce(0.0f);

        for (uint32_t j : m_NeighborScratch)
        {
            if (j == i) continue;

            const FluidParticle& pj = m_Particles[j];
            glm::vec3 rVec = pi.position - pj.position;
            float r = glm::length(rVec);
            if (r <= 0.0f || r > m_SmoothingRadius) continue;

            
            
            float pressureTerm = pj.mass *
                (pi.pressure + pj.pressure) / (2.0f * pj.density);
            pressureForce -= pressureTerm * SPHKernels::SpikyGradient(rVec, r, m_SmoothingRadius, m_SpikyCoefficient);

            glm::vec3 velDiff = pj.velocity - pi.velocity;
            viscosityForce += m_Viscosity * pj.mass * (velDiff / pj.density) *
                SPHKernels::ViscosityLaplacian(r, m_SmoothingRadius, m_ViscosityCoefficient);
        }

        glm::vec3 gravityForce = pi.density * m_Gravity;
        pi.force = pressureForce + viscosityForce + gravityForce;
    }
}

void ParticleSandbox::UpdateKernelConstants()
{
    if (m_KernelRadius == m_SmoothingRadius)
        return;

    m_KernelRadius = m_SmoothingRadius;
    m_Poly6Coefficient = 315.0f /
        (64.0f * glm::pi<float>() * std::pow(m_SmoothingRadius, 9.0f));
    m_SpikyCoefficient = -45.0f /
        (glm::pi<float>() * std::pow(m_SmoothingRadius, 6.0f));
    m_ViscosityCoefficient = 45.0f /
        (glm::pi<float>() * std::pow(m_SmoothingRadius, 6.0f));
}

void ParticleSandbox::Integrate(FluidParticle& particle, float deltaTime)
{
    glm::vec3 acceleration = particle.force / particle.density;
    particle.velocity += acceleration * deltaTime;
    particle.velocity *= m_LinearDamping;

    float speed = glm::length(particle.velocity);
    if (speed > m_MaxSpeed)
        particle.velocity *= (m_MaxSpeed / speed);

    particle.position += particle.velocity * deltaTime;
}

void ParticleSandbox::ResolveBoxCollision(FluidParticle& particle)
{
    glm::vec3 minB = m_BoxCenter - m_BoxHalfExtents;
    glm::vec3 maxB = m_BoxCenter + m_BoxHalfExtents;

    for (int axis = 0; axis < 3; ++axis)
    {
        if (particle.position[axis] < minB[axis] + particle.radius)
        {
            particle.position[axis] = minB[axis] + particle.radius;
            particle.velocity[axis] *= -m_Restitution;
        }
        else if (particle.position[axis] > maxB[axis] - particle.radius)
        {
            particle.position[axis] = maxB[axis] - particle.radius;
            particle.velocity[axis] *= -m_Restitution;
        }
    }
}

void ParticleSandbox::Update(float deltaTime)
{
    UpdateKernelConstants();
    constexpr float kFixedStep = 1.0f / 120.0f;
    constexpr int kMaxSubsteps = 4;

    m_Accumulator += deltaTime;
    int substeps = 0;

    while (m_Accumulator >= kFixedStep && substeps < kMaxSubsteps)
    {
        BuildNeighborGrid();
        ComputeDensityPressure();
        ComputeForces();

        for (auto& p : m_Particles)
        {
            Integrate(p, kFixedStep);
            ResolveBoxCollision(p);
        }

        m_Accumulator -= kFixedStep;
        ++substeps;
    }
}

void ParticleSandbox::SetParticleMaterial(Shader& shader) const
{
    
    
    
    
    
    
    
    
    
    
    
    
    shader.SetUniform3fv("u_Material.ambient", m_Material.ambient);
    shader.SetUniform3fv("u_Material.diffuse", m_Material.diffuse);
    shader.SetUniform3fv("u_Material.specular", m_Material.specular);
    shader.SetUniform3fv("u_Material.emissive", m_Material.emissive);
    shader.SetUniform1f("u_Material.shininess", m_Material.shininess);
    shader.SetUniform1i("u_Material.isEmissive", m_Material.isEmissive ? 1 : 0);
}

void ParticleSandbox::Draw(Shader& shader)
{
    if (m_Particles.empty() || !m_ParticleMesh)
        return;

    
    
    
    SetParticleMaterial(shader);

    m_InstanceScratch.clear();
    m_InstanceScratch.reserve(m_Particles.size() * 4);
    for (const auto& p : m_Particles)
    {
        m_InstanceScratch.push_back(p.position.x);
        m_InstanceScratch.push_back(p.position.y);
        m_InstanceScratch.push_back(p.position.z);
        
        
        
        m_InstanceScratch.push_back(p.radius * m_RenderScale);
    }

    m_ParticleMesh->UpdateInstances(m_InstanceScratch);
    m_ParticleMesh->DrawInstanced(shader);
}

const std::vector<FluidParticle>& ParticleSandbox::GetParticles() const
{
    return m_Particles;
}

void ParticleSandbox::GetBounds(glm::vec3& outMin, glm::vec3& outMax) const
{
    if (m_Particles.empty())
    {
        outMin = outMax = glm::vec3(0.0f);
        return;
    }

    outMin = outMax = m_Particles[0].position;
    for (const auto& p : m_Particles)
    {
        outMin = glm::min(outMin, p.position);
        outMax = glm::max(outMax, p.position);
    }
}

void ParticleSandbox::PrintDebugInfo() const
{
    std::cout << "--- ParticleSandbox debug ---" << std::endl;
    std::cout << "  Particle count: " << m_Particles.size() << std::endl;

    if (m_Particles.empty())
    {
        std::cout << "  No particles exist - nothing to draw. Check the "
            "particleCount argument passed to the ParticleSandbox constructor."
            << std::endl;
        return;
    }

    const FluidParticle& p0 = m_Particles[0];
    std::cout << "  particle[0].position = ("
        << p0.position.x << ", " << p0.position.y << ", " << p0.position.z << ")" << std::endl;
    std::cout << "  particle[0].velocity = ("
        << p0.velocity.x << ", " << p0.velocity.y << ", " << p0.velocity.z << ")" << std::endl;
    std::cout << "  particle[0].density/pressure/mass/radius = "
        << p0.density << " / " << p0.pressure << " / " << p0.mass << " / " << p0.radius << std::endl;
    std::cout << "  effective render diameter = " << (p0.radius * m_RenderScale * 2.0f)
        << " world units (m_RenderScale = " << m_RenderScale << ")" << std::endl;

    
    
    bool hasNaN = glm::any(glm::isnan(p0.position)) || glm::any(glm::isinf(p0.position));
    if (hasNaN)
        std::cout << "  WARNING: particle[0].position contains NaN/Inf - the "
        "simulation has diverged. Try lowering m_GasConstant or increasing "
        "m_Viscosity/damping." << std::endl;

    std::cout << "  Material being uploaded to particle shader:" << std::endl;
    std::cout << "    ambient  = (" << m_Material.ambient.x << ", " << m_Material.ambient.y << ", " << m_Material.ambient.z << ")" << std::endl;
    std::cout << "    diffuse  = (" << m_Material.diffuse.x << ", " << m_Material.diffuse.y << ", " << m_Material.diffuse.z << ")" << std::endl;
    std::cout << "    specular = (" << m_Material.specular.x << ", " << m_Material.specular.y << ", " << m_Material.specular.z << ")" << std::endl;
    std::cout << "    isEmissive = " << (m_Material.isEmissive ? "true" : "false") << std::endl;

    if (!m_Material.isEmissive &&
        m_Material.ambient == glm::vec3(0.0f) &&
        m_Material.diffuse == glm::vec3(0.0f) &&
        m_Material.specular == glm::vec3(0.0f))
    {
        std::cout << "  WARNING: material is fully black and non-emissive - "
            "particles WILL render as invisible regardless of position/scale. "
            "This is the classic symptom of SetParticleMaterial() never being "
            "called on the particle shader before the draw call." << std::endl;
    }
    std::cout << "------------------------------" << std::endl;
}
