#version 330 core

out vec4 out_color;

in vec3 vColor;
in vec2 texCoords;

uniform vec4 u_color;

uniform sampler2D ourTexture;

void main()
{
	out_color = texture(ourTexture, texCoords);
};
