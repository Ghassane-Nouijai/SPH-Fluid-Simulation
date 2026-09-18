#pragma once

#include <memory>
#include "IRenderable.h"
#include "PhysicsObject.h"
#include "Shader.h"
#include "Materials.h"

class SimObject
{
public:
	SimObject(std::unique_ptr<IRenderable> renderable,
		PhysicsObject physics,
		Material material = Material::Rubber())
		: m_Renderable(std::move(renderable))
		, m_PhysicsObject(std::move(physics))
		, m_Material(material)
	{
	}

	void Draw(Shader& shader)
	{
		shader.SetUniform3fv("u_Material.ambient", m_Material.ambient);
		shader.SetUniform3fv("u_Material.diffuse", m_Material.diffuse);
		shader.SetUniform3fv("u_Material.specular", m_Material.specular);
		shader.SetUniform3fv("u_Material.emissive", m_Material.emissive);
		shader.SetUniform1f("u_Material.shininess", m_Material.shininess);
		shader.SetUniform1i("u_Material.isEmissive", m_Material.isEmissive ? 1 : 0);

		if (m_Material.isEmissive)
		{
			shader.SetUniform3fv("u_Light.position", m_PhysicsObject.getPosition());
			shader.SetUniform3fv("u_Light.ambient", m_Material.emissive * 0.2f);
			shader.SetUniform3fv("u_Light.diffuse", m_Material.emissive * 0.8f);
			shader.SetUniform3fv("u_Light.specular", m_Material.emissive);
		}

		m_Renderable->Draw(shader,
			m_PhysicsObject.getPosition(),
			m_PhysicsObject.getOrientation());
	}

	const Material& GetMaterial()     const { return m_Material; }
	PhysicsObject& GetPhysicsObject() { return m_PhysicsObject; }

private:
	std::unique_ptr<IRenderable> m_Renderable;
	PhysicsObject                m_PhysicsObject;
	Material                     m_Material;
};