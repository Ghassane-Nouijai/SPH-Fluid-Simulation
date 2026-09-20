#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aInstance; 

uniform mat4 projection;
uniform mat4 view;

out vec3 v_FragPos;
out vec3 v_Normal;

void main()
{
    
    
    
    vec3 worldPos = aPos * aInstance.w + aInstance.xyz;

    v_FragPos = worldPos;
    v_Normal = aNormal;

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

uniform Material u_Material;
uniform Light u_Light;
uniform vec3 u_ViewPos;

out vec4 FragColor;

void main()
{
    if (u_Material.isEmissive)
    {
        FragColor = vec4(u_Material.emissive, 1.0);
        return;
    }

    vec3 normal   = normalize(v_Normal);
    vec3 lightDir = normalize(u_Light.position - v_FragPos);
    vec3 viewDir  = normalize(u_ViewPos - v_FragPos);
    vec3 halfway  = normalize(lightDir + viewDir);

    vec3 ambient = u_Light.ambient * u_Material.ambient;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = u_Light.diffuse * diff * u_Material.diffuse;

    float spec = pow(max(dot(normal, halfway), 0.0), u_Material.shininess);
    vec3 specular = u_Light.specular * spec * u_Material.specular;

    FragColor = vec4(ambient + diffuse + specular, 1.0);
}