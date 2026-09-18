#shader vertex
#version 330 core
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 v_FragPos;
out vec3 v_Normal;

void main()
{
    vec4 worldPos = model * vec4(a_Position, 1.0);
    v_FragPos     = vec3(worldPos);
    v_Normal      = mat3(transpose(inverse(model))) * a_Normal;
    gl_Position   = projection * view * worldPos;
}

#shader fragment
#version 330 core

in vec3 v_FragPos;
in vec3 v_Normal;

struct Material {
    vec3  ambient;
    vec3  diffuse;
    vec3  specular;
    vec3  emissive;    
    float shininess;
    bool  isEmissive;
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material u_Material;
uniform Light    u_Light;
uniform vec3     u_ViewPos;

out vec4 FragColor;

void main()
{
    if (u_Material.isEmissive)
    {
        FragColor = vec4(u_Material.emissive, 1.0);
        return;
    }

    vec3 norm     = normalize(v_Normal);
    vec3 lightDir = normalize(u_Light.position - v_FragPos);
    vec3 viewDir  = normalize(u_ViewPos - v_FragPos);
    vec3 halfway  = normalize(lightDir + viewDir);

    // Ambient
    vec3 ambient  = u_Light.ambient * u_Material.ambient;

    // Diffuse
    float diff    = max(dot(norm, lightDir), 0.0);
    vec3 diffuse  = u_Light.diffuse * diff * u_Material.diffuse;

    // Specular (Blinn-Phong)
    float spec    = pow(max(dot(halfway, norm), 0.0), u_Material.shininess);
    vec3 specular = u_Light.specular * spec * u_Material.specular;

    // Emissive term added on top of lighting (for non-light-source glows)
    vec3 emissive = u_Material.emissive;

    FragColor = vec4(ambient + diffuse + specular + emissive, 1.0);
}