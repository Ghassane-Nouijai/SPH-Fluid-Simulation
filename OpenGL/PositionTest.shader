#shader vertex
#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec4 ColourPos;

void main()
{
	gl_Position = projection * view * model * vec4(position, 1.0);
	ColourPos = model * vec4(position, 1.0);
};

#shader fragment
#version 330 core

layout (location = 0) out vec4 colour;
in vec4 ColourPos;

void main()
{
   colour = vec4(ColourPos);
};