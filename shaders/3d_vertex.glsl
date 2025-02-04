#version 460 core

layout (location = 0) in vec3 aPos; // Vertex positions
layout (location = 1) in vec3 aTex; // Vertex positions

// Uniforms for transformation
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
