#include "ParticleSandbox.h"

#include <random>
#include <algorithm>

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
    , m_Material(Material::Rubber())
    , m_Restitution(0.3f)
    , m_LinearDamping(0.995f)
    , m_IdentityOrientation(1.0f, 0.0f, 0.0f, 0.0f)
{
    // Base mesh is a UNIT sphere (radius 1.0). Actual on-screen particle size
    // comes entirely from the per-instance scale fed in Draw() (= particle
    // .radius), so this stays independent of whatever radius you simulate
    // with. Low precision because we may draw thousands of these per frame.
    m_ParticleMesh = std::make_unique<Sphere>(1.0f, 8);

    CreateParticles(particleCount);
    InitializeParticleMasses();

    m_NeighborScratch.reserve(128);
    m_InstanceScratch.reserve(particleCount * 4);
}

void ParticleSandbox::CreateParticles(std::size_t particleCount)
{
    m_Particles.clear();
    m_Particles.reserve(particleCount);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> jitter(-0.02f, 0.02f);

    // Pack particles into a cube-ish block inside the box, spaced at ~1 particle
    // radius apart so the initial density is close to rest density and the
    // simulation doesn't explode on frame 1.
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
                p.mass = 1.0f; // placeholder - corrected by InitializeParticleMasses()
                p.radius = 0.08f;

                m_Particles.push_back(p);
                ++created;
            }
}

void ParticleSandbox::InitializeParticleMasses()
{
    // Why this exists:
    // Density is estimated as rho_i = sum_j mass_j * Poly6(r_ij, h). With
    // mass left at the CreateParticles() placeholder of 1.0, that sum comes
    // out far below m_RestDensity (1000) for any reasonable packing - so
    // pi.pressure = max(k * (density - rest), 0) is clamped to zero on every
    // frame. Zero pressure means the Spiky-gradient repulsion term in
    // ComputeForces() never contributes anything, so nothing pushes
    // particles apart sideways: they just free-fall under gravity and
    // viscosity and settle into whatever shape they were spawned in (hence
    // the "uniform cube" that never splashes).
    //
    // Fix: measure the density the *current* packing produces with mass = 1,
    // then rescale mass uniformly so the average measured density equals
    // m_RestDensity. This is the standard SPH "mass calibration" step and
    // only needs to run once, right after the particles are placed.
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

void ParticleSandbox::BuildNeighborGrid()
{
    std::vector<glm::vec3> positions;
    positions.reserve(m_Particles.size());
    for (auto& p : m_Particles)
        positions.push_back(p.position);

    m_Grid.Build(positions, m_SmoothingRadius);
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
            density += pj.mass * SPHKernels::Poly6(r, m_SmoothingRadius);
        }

        pi.density = std::max(density, 1e-6f);
        // Ideal-gas-like equation of state (Muller et al.). Clamp to zero so
        // particles below rest density don't get pulled together (no "tension").
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

            // Symmetric pressure term avoids attraction/repulsion imbalance
            // between particle pairs (Muller et al. 2003, eq. 10).
            float pressureTerm = pj.mass *
                (pi.pressure + pj.pressure) / (2.0f * pj.density);
            pressureForce -= pressureTerm * SPHKernels::SpikyGradient(rVec, r, m_SmoothingRadius);

            glm::vec3 velDiff = pj.velocity - pi.velocity;
            viscosityForce += m_Viscosity * pj.mass * (velDiff / pj.density) *
                SPHKernels::ViscosityLaplacian(r, m_SmoothingRadius);
        }

        glm::vec3 gravityForce = pi.density * m_Gravity;
        pi.force = pressureForce + viscosityForce + gravityForce;
    }
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
    // Fixed sub-stepping keeps the solver stable at typical smoothing radii;
    // a single large dt at 60fps can blow up the pressure force. Tune
    // subStepCount / maxSubDt if you see jitter or particles flying out.
    constexpr int   kMaxSubSteps = 4;
    constexpr float kMaxSubDt = 1.0f / 240.0f;

    int subSteps = std::clamp(static_cast<int>(std::ceil(deltaTime / kMaxSubDt)), 1, kMaxSubSteps);
    float subDt = deltaTime / static_cast<float>(subSteps);

    for (int step = 0; step < subSteps; ++step)
    {
        BuildNeighborGrid();
        ComputeDensityPressure();
        ComputeForces();

        for (auto& particle : m_Particles)
        {
            Integrate(particle, subDt);
            ResolveBoxCollision(particle);
        }
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
    SetParticleMaterial(shader);

    // Pack (position.xyz, radius) per particle into a flat float buffer and
    // hand it to the mesh's generic instancing path - this replaces what
    // used to be one glDrawElements() call *per particle* with a single
    // glDrawElementsInstanced() call for the whole simulation.
    m_InstanceScratch.clear();
    for (const auto& particle : m_Particles)
    {
        m_InstanceScratch.push_back(particle.position.x);
        m_InstanceScratch.push_back(particle.position.y);
        m_InstanceScratch.push_back(particle.position.z);
        m_InstanceScratch.push_back(particle.radius);
    }

    m_ParticleMesh->UpdateInstances(m_InstanceScratch);
    m_ParticleMesh->DrawInstanced(shader);
}

const std::vector<FluidParticle>& ParticleSandbox::GetParticles() const
{
    return m_Particles;
}
