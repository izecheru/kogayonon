#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D texture_normal;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_metallic_roughness;

uniform vec3 light_pos;
uniform vec3 view_pos;
uniform vec3 light_color;

void main()
{
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * light_color;

    FragColor = texture(texture_normal, TexCoords);
}  