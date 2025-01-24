//#shader fragment
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
