#version 330 core

layout(location = 0) in vec2 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec2 texC;

out vec3 vColor;
out vec2 texCoords;

void main()
{
	gl_Position = vec4(position, 0, 1);
	vColor = color;
	texCoords = texC;
};
