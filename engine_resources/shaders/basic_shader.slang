#include "structures.slang"
#include "utils.slang"

struct VertexInput
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
};

struct VertexOut
{
  float4 position : SV_Position;
  float2 fragTexCoord : TEXCOORD0;
};

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
VertexOut vertexMain( VertexInput input )
{
  VertexOut output;

  float4 pos = float4( input.position, 1.0 );
  pos = mul( push.modelMatrix, pos );
  pos = mul( camera.view, pos );
  pos = mul( camera.projection, pos );

  output.position = pos;
  output.fragTexCoord = input.uv;

  return output;
}

[shader( "fragment" )]
FragmentOutput fragmentMain( FragmentInput input )
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

