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
uniform mat4 lightVP;

// Outputs
out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 ShadowCoord;
out vec3 ViewPos;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(instanceMatrix)));

    FragPos = vec3(instanceMatrix * vec4(aPos,1.0f));
    Normal = normalize(normalMatrix * aNormal);
    TexCoord = aTexCoord;
    ShadowCoord = lightVP  * vec4(FragPos,1.0f);
    gl_Position = projection * view * vec4(FragPos,1.0f);

    // calculate the view pos by inversing the view matrix
    mat4 viewInverse = inverse(view); 
    // now get the x, y, z
    ViewPos = vec3(viewInverse[3][0],viewInverse[3][1],viewInverse[3][2]);
}
