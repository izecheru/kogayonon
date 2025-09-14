#version 460 core

layout (binding = 1) uniform sampler2D uTexture;

in vec2 TexCoord;
out vec4 FragColor;

void main()
{
    FragColor = texture(uTexture, TexCoord);
}
