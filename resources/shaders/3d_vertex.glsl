#version 460 core

// Vertex attributes
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

// Instance attributes
layout(location = 3) in mat4 instanceMatrix;
layout(location = 7) in int aEntityID;

// Uniforms
uniform mat4 view;
uniform mat4 projection;
uniform bool instanced;
uniform mat4 model;

// Outputs
out vec2 TexCoord;
out flat int v_EntityID; // flat so it is not interpolated

void main()
{
    if(instanced){
        gl_Position = projection * view * instanceMatrix * vec4(aPos, 1.0);
    }
    else{
        gl_Position = projection * view * model * vec4(aPos, 1.0);
    }
    v_EntityID = aEntityID;
    TexCoord = aTexCoord;
}