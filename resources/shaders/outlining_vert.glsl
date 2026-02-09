#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 5) in uint aSelected;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 instanceMatrix;

out vec3 FragPos;

void main()
{
    float outlineWidth = 0.055;

    float scaleX = length(instanceMatrix[0].xyz);
    float scaleY = length(instanceMatrix[1].xyz);
    float scaleZ = length(instanceMatrix[2].xyz);

    vec3 scaledOutlineWidth = outlineWidth / vec3(scaleX, scaleY, scaleZ);
    vec3 outlineOffset = aNormal * scaledOutlineWidth;
    vec3 newPos = aPos + outlineOffset;
    vec4 worldPos = instanceMatrix * vec4(newPos, 1.0);

    FragPos = vec4(instanceMatrix * vec4(newPos,1.0)).xyz;
    gl_Position = projection * view * worldPos;
}
