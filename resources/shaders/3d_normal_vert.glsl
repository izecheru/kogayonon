#version 460 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in uint aSelected;
// layout(location = 4) in int aEntityId;



uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightVP;
// this no longer is instanced but i'll leave the variable name the same
uniform mat4 instanceMatrix;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 ShadowCoord;
out vec3 ViewPos;
flat out uint Selected;

void main()
{
  FragPos = vec3(instanceMatrix * vec4(aPos,1.0f));
  mat3 normalMatrix = transpose(inverse(mat3(instanceMatrix)));
  Normal = normalize(normalMatrix * aNormal);
  TexCoord = aTexCoord;
  ShadowCoord = lightVP  * vec4(FragPos,1.0f);
  gl_Position = projection * view * vec4(FragPos,1.0f);

  // calculate the view pos by inversing the view matrix
  mat4 viewInverse = inverse(view); 
  // now get the x, y, z
  ViewPos = vec3(viewInverse[3][0],viewInverse[3][1],viewInverse[3][2]);
  Selected = aSelected;
}