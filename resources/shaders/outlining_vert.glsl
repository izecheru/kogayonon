#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in uint aSelected;

uniform mat4 view;
uniform mat4 projection;
uniform mat4 instanceMatrix;

out vec3 FragPos;

void main()
{
  float outlineScale = 0.009;
  FragPos = vec3( instanceMatrix  * vec4(aPos + aNormal * outlineScale,1.0));
  gl_Position = projection * view * vec4(FragPos,1.0);
}
