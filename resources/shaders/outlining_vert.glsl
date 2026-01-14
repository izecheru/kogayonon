#version 460 core

// Vertex attributes
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// Instance attributes
layout(location = 3) in mat4 instanceMatrix;

// Uniforms
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;

void main()
{
  float outlineScale = 0.05;
  vec3 scaledPos = aPos + aNormal * outlineScale;
  FragPos = vec3(instanceMatrix * vec4(scaledPos,1.0));
  gl_Position = projection * view * vec4(FragPos,1.0);
}
