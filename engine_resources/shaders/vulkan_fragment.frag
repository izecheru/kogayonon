#version 460

#extension GL_EXT_nonuniform_qualifier : require

layout( location = 0 ) in vec2 fragTexCoord;
layout( location = 0 ) out vec4 outColor;

// TODO(kogayonon) this should be worked on
struct MaterialDescription
{
  int normalIndex;
  int specularIndex;
  int diffuseIndex;
};

layout( set = 1, binding = 0 ) uniform sampler2D inBindlessTextures[];

layout( set = 2, binding = 0 ) buffer MaterialsBuffer
{
  MaterialDescription materials[];
};

layout( push_constant ) uniform Push
{
  mat4 modelMatrix;
  int materialIndex;
};

MaterialDescription getMaterial( int materialIndex )
{
  return materials[materialIndex];
}

void main()
{
  MaterialDescription material = getMaterial( materialIndex );

  vec4 diffuseColor = vec4( 1.0 );
  vec3 normalColor = vec3( 0.5f, 0.5f, 1.0f );

  if ( material.diffuseIndex != -1 )
  {
    diffuseColor = texture( inBindlessTextures[nonuniformEXT( material.diffuseIndex )], fragTexCoord );
  }

  if ( material.normalIndex != -1 )
  {
    normalColor = texture( inBindlessTextures[nonuniformEXT( material.normalIndex )], fragTexCoord ).xyz;
  }

  outColor = vec4( 0.4f, 0.4f, 0.4f, 0.5f );
}

