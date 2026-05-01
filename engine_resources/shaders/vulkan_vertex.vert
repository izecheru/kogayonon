#version 460

#extension GL_EXT_scalar_block_layout : require
// INPUT
layout( location = 0 ) in vec3 inPosition;
layout( location = 1 ) in float inUv_X;
layout( location = 2 ) in vec3 inNormal;
layout( location = 3 ) in float inUv_Y;

// OUT
layout( location = 0 ) out vec2 fragTexCoord;

layout( set = 0, binding = 0 ) uniform CameraBuffer
{
  mat4 view;
  mat4 proj;
};

// PUSH CONSTANT
layout( scalar, push_constant ) uniform Push
{
  mat4 modelMatrix;
  int materialIndex;
};

void main()
{
  gl_Position = proj * view * modelMatrix * vec4( inPosition, 1.0 );
  fragTexCoord = vec2( inUv_X, inUv_Y );
}

