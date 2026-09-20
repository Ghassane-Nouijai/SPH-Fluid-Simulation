#include "ParticleSandbox.h"

#include <random>
#include <algorithm>
#include <execution>
#include <numeric>
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
    m_ParticleMesh = std::make_unique<Sphere>(1.0f, 8);

    CreateParticles(particleCount);
    InitializeParticleMasses();

    m_Indices.resize(m_Particles.size());
    std::iota(m_Indices.begin(), m_Indices.end(), std::size_t{ 0 });

    m_PositionScratch.reserve(particleCount);
    m_InstanceScratch.reserve(particleCount * 5); // 5 floats/instance now
}

void ParticleSandbox::CreateParticles(std::size_t particleCount)
{
    m_Particles.clear();
    m_Particles.reserve(particleCount);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> jitter(-0.02f, 0.02f);

    float spacing = m_SmoothingRadius * 0.5f;
    int perAxis = static_cast<int>(std::ceil(std::cbrt(static_cast<double>(particleCount))));

    glm::vec3 origin = m_BoxCenter + glm::vec3(4.0f, -3.0f, 2.0f) - glm::vec3(perAxis * spacing * 0.5f);

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
                p.mass = 1.0f; // placeholder corrected by InitializeParticleMasses()
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
            density += 1.0f * SPHKernels::Poly6(r, m_SmoothingRadius); // mass = 1
        }
        totalDensity += density;
    }

    float avgDensityWithUnitMass = totalDensity / static_cast<float>(m_Particles.size());
    float calibratedMass = m_RestDensity / std::max(avgDensityWithUnitMass, 1e-6f);

    for (auto& p : m_Particles)
        p.mass = calibratedMass;
}

void ParticleSandbox::UpdateKernelConstants()
{
    if (m_KernelRadius == m_SmoothingRadius)
        return;

    m_KernelRadius = m_SmoothingRadius;
    const float h = m_SmoothingRadius;

    m_Poly6Coefficient = 315.0f / (64.0f * glm::pi<float>() * std::pow(h, 9));
    m_SpikyCoefficient = -45.0f / (glm::pi<float>() * std::pow(h, 6));
    m_ViscosityCoefficient = 45.0f / (glm::pi<float>() * std::pow(h, 6));
}

void ParticleSandbox::BuildNeighborGrid()
{
    m_PositionScratch.resize(m_Particles.size());
    for (std::size_t i = 0; i < m_Particles.size(); ++i)
        m_PositionScratch[i] = m_Particles[i].position;

    m_Grid.Build(m_PositionScratch, m_SmoothingRadius);
}

void ParticleSandbox::ComputeDensityPressure()
{
    const float h = m_SmoothingRadius;
    const float poly6Coeff = m_Poly6Coefficient;
    std::for_each(std::execution::par, m_Indices.begin(), m_Indices.end(),
        [this, h, poly6Coeff](std::size_t i)
        {
            thread_local std::vector<uint32_t> neighbors;
            neighbors.clear();

            FluidParticle& pi = m_Particles[i];
            m_Grid.Query(pi.position, neighbors);

            float density = 0.0f;
            for (uint32_t j : neighbors)
            {
                const FluidParticle& pj = m_Particles[j];
                float r = glm::length(pi.position - pj.position);
                density += pj.mass * SPHKernels::Poly6(r, h, poly6Coeff);
            }

            pi.density = std::max(density, 1e-6f);
            
            pi.pressure = std::max(m_GasConstant * (pi.density - m_RestDensity), 0.0f);
        });
}

void ParticleSandbox::ComputeForces()
{
    const float h = m_SmoothingRadius;
    const float spikyCoeff = m_SpikyCoefficient;
    const float viscCoeff = m_ViscosityCoefficient;

    std::for_each(std::execution::par, m_Indices.begin(), m_Indices.end(),
        [this, h, spikyCoeff, viscCoeff](std::size_t i)
        {
            thread_local std::vector<uint32_t> neighbors;
            neighbors.clear();

            FluidParticle& pi = m_Particles[i];
            m_Grid.Query(pi.position, neighbors);

            glm::vec3 pressureForce(0.0f);
            glm::vec3 viscosityForce(0.0f);

            for (uint32_t j : neighbors)
            {
                if (j == i) continue;

                const FluidParticle& pj = m_Particles[j];
                glm::vec3 rVec = pi.position - pj.position;
                float r = glm::length(rVec);
                if (r <= 0.0f || r > h) continue;

                float pressureTerm = pj.mass *
                    (pi.pressure + pj.pressure) / (2.0f * pj.density);
                pressureForce -= pressureTerm * SPHKernels::SpikyGradient(rVec, r, h, spikyCoeff);

                glm::vec3 velDiff = pj.velocity - pi.velocity;
                viscosityForce += m_Viscosity * pj.mass * (velDiff / pj.density) *
                    SPHKernels::ViscosityLaplacian(r, h, viscCoeff);
            }

            glm::vec3 gravityForce = pi.density * m_Gravity;
            pi.force = pressureForce + viscosityForce + gravityForce;
        });
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

    constexpr float kFixedTimestep = 1.0f / 120.0f;
    constexpr int kMaxSubsteps = 4; // hard cap - see comment below

    m_Accumulator += deltaTime;

    int substeps = 0;
    while (m_Accumulator >= kFixedTimestep && substeps < kMaxSubsteps)
    {
        BuildNeighborGrid();
        ComputeDensityPressure();
        ComputeForces();

        std::for_each(std::execution::par, m_Indices.begin(), m_Indices.end(),
            [this](std::size_t i)
            {
                Integrate(m_Particles[i], kFixedTimestep);
                ResolveBoxCollision(m_Particles[i]);
            });

        m_Accumulator -= kFixedTimestep;
        ++substeps;
    }
    if (substeps == kMaxSubsteps)
        m_Accumulator = 0.0f;
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
    SetParticleMaterial(shader);
    shader.SetUniform1f("u_SpeedColorReference", m_SpeedColorReference);

    constexpr std::size_t kFloatsPerInstance = 5; // pos.xyz, renderScale, speed
    m_InstanceScratch.clear();

    for (const auto& p : m_Particles)
    {
        float speed = glm::length(p.velocity);
        m_InstanceScratch.push_back(p.position.x);
        m_InstanceScratch.push_back(p.position.y);
        m_InstanceScratch.push_back(p.position.z);
        m_InstanceScratch.push_back(p.radius * m_RenderScale);
        m_InstanceScratch.push_back(speed);
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
    std::cout << "--- ParticleSandbox debug ---\n";
    std::cout << "  Particle count: " << m_Particles.size() << "\n";

    if (m_Particles.empty())
    {
        std::cout << "  (no particles - nothing else to report)\n";
        std::cout << "------------------------------\n";
        return;
    }

    const FluidParticle& p0 = m_Particles[0];
    float speed0 = glm::length(p0.velocity);

    std::cout << "  particle[0].position = (" << p0.position.x << ", " << p0.position.y << ", " << p0.position.z << ")\n";
    std::cout << "  particle[0].velocity = (" << p0.velocity.x << ", " << p0.velocity.y << ", " << p0.velocity.z << ")\n";
    std::cout << "  particle[0].density/pressure/mass/radius = "
        << p0.density << " / " << p0.pressure << " / " << p0.mass << " / " << p0.radius << "\n";
    std::cout << "  particle[0].speed = " << speed0
        << "  (speed color ref = " << m_SpeedColorReference << ")\n";

    float renderDiameter = p0.radius * m_RenderScale * 2.0f;
    std::cout << "  effective render diameter = " << renderDiameter
        << " world units (m_RenderScale = " << m_RenderScale << ")\n";

    if (m_InstanceScratch.size() >= 5)
    {
        std::cout << "  instance[0] uploaded to GPU = ("
            << m_InstanceScratch[0] << ", " << m_InstanceScratch[1] << ", "
            << m_InstanceScratch[2] << ", scale=" << m_InstanceScratch[3]
            << ", speed=" << m_InstanceScratch[4] << ")\n";
    }
    else
    {
        std::cout << "  [WARNING] m_InstanceScratch has fewer than 5 floats - "
            "Draw() may not have run yet this frame.\n";
    }

    std::cout << "  Material being uploaded to particle shader:\n";
    std::cout << "    ambient  = (" << m_Material.ambient.x << ", " << m_Material.ambient.y << ", " << m_Material.ambient.z << ")\n";
    std::cout << "    diffuse  = (" << m_Material.diffuse.x << ", " << m_Material.diffuse.y << ", " << m_Material.diffuse.z << ")\n";
    std::cout << "    specular = (" << m_Material.specular.x << ", " << m_Material.specular.y << ", " << m_Material.specular.z << ")\n";
    std::cout << "    isEmissive = " << (m_Material.isEmissive ? "true" : "false") << "\n";
    std::cout << "------------------------------\n";
}
