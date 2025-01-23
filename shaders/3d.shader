#shader vertex
#version 460 core

// Positions/Coordinates
layout (location = 0) in vec3 aPos;
// Colors
layout (location = 1) in vec3 aColor;

// Outputs to the Fragment Shader
out vec3 vertexColor; // Pass color to fragment shader

// Uniforms for transformation
uniform float scale;
uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    // Scale the vertex positions
    vec4 scaledPosition = vec4(aPos * scale, 1.0);

    // Transform the positions with model, view, and projection matrices
    gl_Position = proj * view * model * scaledPosition;

    // Pass the vertex color to the fragment shader
    vertexColor = aColor;
}

#shader fragment
#version 460 core

// Outputs the final color
out vec4 FragColor;

// Inputs from the Vertex Shader
in vec3 vertexColor;   // Color passed from vertex shader

void main()
{
    // Use the vertex color directly as the output color
    FragColor = vec4(vertexColor, 1.0);
}
