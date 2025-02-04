
#version 460 core

in vec2 TexCoord; // Texture coordinate passed from vertex shader
out vec4 FragColor; // Output color

void main()
{
    // Output a simple green color
    FragColor = vec4(0.0f, 1.0f, 0.0f, 1.0f); // RGBA (Green color)
}
