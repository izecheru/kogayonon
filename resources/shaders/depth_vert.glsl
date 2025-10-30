#version 460 core

// Vertex attributes
layout(location = 0) in vec3 aPos;

// Instance attributes
layout(location = 3) in mat4 instanceMatrix;

// Uniforms
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

// Outputs
out vec3 FragPos;

void main()
{
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
    FragPos = vec3(instanceMatrix*vec4(aPos,1.0));
}
