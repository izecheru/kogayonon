#include "structures.slang"
#include "utils.slang"

struct VertexInput
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
};

struct VertexOutput
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

[[vk::binding( 0, 0 )]]
ConstantBuffer<Camera> camera;

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

[shader( "vertex" )]
VertexOutput vertexMain( VertexInput input )
{
  VertexOutput output;

  output.position =
    mul( camera.projection, mul( camera.view, mul( push.modelMatrix, float4( input.position, 1.0f ) ) ) );
  output.fragTexCoord = input.uv;

  return output;
}

[shader( "fragment" )]
FragmentOutput fragmentMain( VertexOutput input )
{
  FragmentOutput output;

  MaterialDescription material = getMaterial( push.materialIndex );

  float4 diffuseColor = float4( 1.0 );
  float3 normalColor = float3( 0.5, 0.5, 1.0 );

  if ( material.diffuseIndex != -1 )
  {
    diffuseColor = bindlessTextures[NonUniformResourceIndex( material.diffuseIndex )].Sample( input.fragTexCoord );
  }

  output.outColor = diffuseColor;
  return output;
}

