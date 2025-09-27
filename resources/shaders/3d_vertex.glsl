#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in mat4 instanceMatrix;

uniform mat4 view;
uniform mat4 projection;
uniform bool instanced;

out vec2 TexCoord;

void main()
{
    if(instanced){
        gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
    }
    else{
        gl_Position = projection * view * vec4(aPos,1.0);
    }
    TexCoord = aTexCoord;
}