#include "ParticleSandbox.h"

#include <algorithm>
#include <cmath>

#include "IRenderable.h"
#include "Shader.h"
#include "Sphere.h"

namespace
{
    constexpr float kParticleRadius = 0.12f;
    constexpr float kParticleMass = 1.0f;
    constexpr float kRestDensity = 1000.0f;
    constexpr unsigned int kSpherePrecision = 16;
}

ParticleSandbox::ParticleSandbox(std::size_t particleCount,
    const glm::vec3& boxCenter,
    const glm::vec3& boxHalfExtents)
    : m_BoxCenter(boxCenter)
    , m_BoxHalfExtents(boxHalfExtents)
    , m_Gravity(0.0f, -9.81f, 0.0f)
    , m_Material(Material::Emissive(glm::vec3(0.08f, 0.45f, 1.0f)))
    , m_Restitution(0.35f)
    , m_LinearDamping(0.998f)
    , m_IdentityOrientation(1.0f, 0.0f, 0.0f, 0.0f)
{
    m_ParticleMesh = std::make_unique<Sphere>(kParticleRadius, kSpherePrecision);
    CreateParticles(particleCount);
}

void ParticleSandbox::CreateParticles(std::size_t particleCount)
{
    const float spacing = kParticleRadius * 2.15f;
    const int columns = static_cast<int>(std::ceil(std::cbrt(static_cast<float>(particleCount))));
    const int rows = columns;

    const glm::vec3 start = m_BoxCenter + glm::vec3(
        -0.5f * static_cast<float>(columns - 1) * spacing,
        0.5f * m_BoxHalfExtents.y,
        -0.5f * static_cast<float>(rows - 1) * spacing);

    m_Particles.reserve(particleCount);

    for (std::size_t index = 0; index < particleCount; ++index)
    {
        const int x = static_cast<int>(index % static_cast<std::size_t>(columns));
        const int z = static_cast<int>((index / static_cast<std::size_t>(columns)) % static_cast<std::size_t>(rows));
        const int y = static_cast<int>(index / static_cast<std::size_t>(columns * rows));

        FluidParticle particle{};
        particle.position = start + glm::vec3(
            static_cast<float>(x) * spacing,
            -static_cast<float>(y) * spacing,
            static_cast<float>(z) * spacing);
        particle.velocity = glm::vec3(0.0f);
        particle.force = glm::vec3(0.0f);
        particle.density = kRestDensity;
        particle.pressure = 0.0f;
        particle.mass = kParticleMass;
        particle.radius = kParticleRadius;

        ResolveBoxCollision(particle);
        m_Particles.push_back(particle);
    }
}

void ParticleSandbox::Update(float deltaTime)
{
    // Clamp protects the simple integrator after a breakpoint or window drag.
    const float clampedDeltaTime = std::min(deltaTime, 1.0f / 60.0f);

    for (FluidParticle& particle : m_Particles)
    {
        Integrate(particle, clampedDeltaTime);
        ResolveBoxCollision(particle);
    }
}

void ParticleSandbox::Integrate(FluidParticle& particle, float deltaTime)
{
    // In a future SPH step, particle.force will also receive pressure and
    // viscosity forces.  For now gravity is the only external acceleration.
    const glm::vec3 acceleration = m_Gravity + particle.force / particle.mass;

    particle.velocity += acceleration * deltaTime;
    particle.velocity *= std::pow(m_LinearDamping, deltaTime * 60.0f);
    particle.position += particle.velocity * deltaTime;
    particle.force = glm::vec3(0.0f);
}

void ParticleSandbox::ResolveBoxCollision(FluidParticle& particle)
{
    const glm::vec3 minimum = m_BoxCenter - m_BoxHalfExtents + glm::vec3(particle.radius);
    const glm::vec3 maximum = m_BoxCenter + m_BoxHalfExtents - glm::vec3(particle.radius);

    for (int axis = 0; axis < 3; ++axis)
    {
        if (particle.position[axis] < minimum[axis])
        {
            particle.position[axis] = minimum[axis];
            if (particle.velocity[axis] < 0.0f)
                particle.velocity[axis] = -particle.velocity[axis] * m_Restitution;
        }
        else if (particle.position[axis] > maximum[axis])
        {
            particle.position[axis] = maximum[axis];
            if (particle.velocity[axis] > 0.0f)
                particle.velocity[axis] = -particle.velocity[axis] * m_Restitution;
        }
    }
}

void ParticleSandbox::Draw(Shader& shader)
{
    SetParticleMaterial(shader);

    for (const FluidParticle& particle : m_Particles)
        m_ParticleMesh->Draw(shader, particle.position, m_IdentityOrientation);
}

const std::vector<FluidParticle>& ParticleSandbox::GetParticles() const
{
    return m_Particles;
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