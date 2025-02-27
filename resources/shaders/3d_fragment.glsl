#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture_normal;

void main()
{
    FragColor = texture(texture_normal, TexCoords);
}
