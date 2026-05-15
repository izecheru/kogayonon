struct FragmentInput
{
  float4 position : SV_Position;
  float2 fragTexCoord : TEXCOORD0;
};

struct FragmentOutput
{
  float4 outColor : SV_Target0;
};

// Check issue 12 (https://github.com/izecheru/kogayonon/issues/12)
struct MaterialDescription
{
  int normalIndex;
  int specularIndex;
  int diffuseIndex;
};

struct PushConstant
{
  float4x4 modelMatrix;
  int materialIndex;
};

[[vk::binding( 0, 1 )]]
Sampler2D bindlessTextures[];

[[vk::push_constant]]
PushConstant push;

[[vk::binding( 0, 2 )]]
StructuredBuffer<MaterialDescription, ScalarDataLayout> materials;

MaterialDescription getMaterial( int materialIndex )
{
  return materials[materialIndex];
}

[shader( "fragment" )]
FragmentOutput main( FragmentInput input )
{
  FragmentOutput output;

  MaterialDescription material = getMaterial( push.materialIndex );

  float4 diffuseColor = float4( 1.0 );
  float3 normalColor = float3( 0.5, 0.5, 1.0 );

  if ( material.diffuseIndex != -1 )
  {
    diffuseColor = bindlessTextures[NonUniformResourceIndex( material.diffuseIndex )].Sample( input.fragTexCoord );
  }

  output.outColor = float4( diffuseColor );
  return output;
}

