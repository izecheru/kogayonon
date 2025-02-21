#version 460 core

out vec4 FragColor;
in vec2 TexCoords;

// Uniform for the diffuse texture
uniform sampler2D texture_diffuse1;

void main()
{
    // Sample the diffuse texture
    vec4 diffuseColor = texture(texture_diffuse1, TexCoords);

    // If the diffuse texture is missing (black or fully transparent), fallback to white
    if (diffuseColor.a == 0.0) {
        diffuseColor = vec4(1.0, 1.0, 1.0, 1.0); // Default to white if texture is missing or fully transparent
    }

    // Set the final fragment color
    FragColor = diffuseColor;
}
