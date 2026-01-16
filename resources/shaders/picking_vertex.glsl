#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 4) in int aEntityId;
layout (location = 5) in mat4 instanceMatrix;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

out flat int v_entityId;

void main()
{
    gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
    v_entityId = aEntityId;
}