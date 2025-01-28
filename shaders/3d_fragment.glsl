//#shader fragment
#version 460 core

// Outputs the final color
out vec4 FragColor;

// Inputs from the Vertex Shader
in vec3 vertexColor;   // Color passed from vertex shader
in vec2 texCoord;

uniform sampler2D tex0;

void main()
{
    // Use the vertex color directly as the output color
    vec4 texColor = texture(tex0, texCoord);
    FragColor = texColor;
}
