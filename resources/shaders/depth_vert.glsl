#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 5) in mat4 instanceMatrix;

// matrix from the light pov, the light is looking at the object
uniform mat4 projection;
uniform mat4 view;

void main()
{
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
}
