#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in uint aSelected;

layout(location = 5) in mat4 instanceMatrix;


uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;

void main()
{
  float outlineScale = 0.038;
  FragPos = vec3( instanceMatrix * vec4(aPos,1.0));
  gl_Position = projection * view * vec4(FragPos + (aNormal * outlineScale),1.0);
}
