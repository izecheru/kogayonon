#version 460 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
// Colors
layout (location = 1) in vec3 aColor;

// Outputs to the Fragment Shader
out vec3 vertexColor; // Pass color to fragment shader

// Uniforms for transformation
uniform mat4 scaleMatrix;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Scale the vertex positions
    vec4 scaledPosition = vec4(aPos , 1.0) * scaleMatrix;

    // Transform the positions with model, view, and projection matrices
    gl_Position = projection * view * model * scaledPosition;

    // Pass the vertex color to the fragment shader
    vertexColor = aColor;
}

