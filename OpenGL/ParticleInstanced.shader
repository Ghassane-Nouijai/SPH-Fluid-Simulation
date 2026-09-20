#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aInstance; // xyz = world position, w = uniform scale
layout(location = 3) in float aSpeed;   // per-particle speed, world units/sec (cosmetic only)

uniform mat4 projection;
uniform mat4 view;

out vec3 v_FragPos;
out vec3 v_Normal;
out float v_Speed;

void main()
{
    // Uniform scale + no per-instance rotation, so normals need no
    // re-transform (a normal matrix would only matter for non-uniform
    // scale or rotation).
    vec3 worldPos = aPos * aInstance.w + aInstance.xyz;

    v_FragPos = worldPos;
    v_Normal = aNormal;
    v_Speed = aSpeed;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}

#shader fragment
#version 330 core

struct Material
{
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    vec3  emissive;
    float shininess;
    bool  isEmissive;
};

struct Light
{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 v_FragPos;
in vec3 v_Normal;
in float v_Speed;

uniform Material u_Material;
uniform Light u_Light;
uniform vec3 u_ViewPos;

// Speed (world units/sec) at which the red tint reaches full designed
// strength. Purely visual - set from ParticleSandbox::m_SpeedColorReference,
// has zero effect on the simulation itself.
uniform float u_SpeedColorReference;

out vec4 FragColor;

void main()
{
    if (u_Material.isEmissive)
    {
        FragColor = vec4(u_Material.emissive, 1.0);
        return;
    }

    // Tint faster-moving particles slightly redder so motion/turbulence is
    // visible at a glance. Capped at 0.6 blend so it stays "slight" rather
    // than overpowering the base water color even at high speed.
    float speedT = clamp(v_Speed / max(u_SpeedColorReference, 1e-4), 0.0, 1.0);
    vec3 fastColor = vec3(1.0, 0.15, 0.1);
    vec3 tintedDiffuse = mix(u_Material.diffuse, fastColor, speedT * 0.6);

    vec3 normal   = normalize(v_Normal);
    vec3 lightDir = normalize(u_Light.position - v_FragPos);
    vec3 viewDir  = normalize(u_ViewPos - v_FragPos);
    vec3 halfway  = normalize(lightDir + viewDir);

    vec3 ambient = u_Light.ambient * u_Material.ambient;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = u_Light.diffuse * diff * tintedDiffuse;

    float spec = pow(max(dot(normal, halfway), 0.0), u_Material.shininess);
    vec3 specular = u_Light.specular * spec * u_Material.specular;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}
