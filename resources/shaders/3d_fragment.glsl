#version 460 core

// Inputs
in vec2 TexCoord;
in flat int v_EntityID;

// Textures
layout(binding = 1) uniform sampler2D u_Texture;

// Outputs
layout(location = 0) out vec4 FragColor;

void main()
{
    FragColor = texture(u_Texture, TexCoord);
}