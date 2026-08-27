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

// descriptor 0 set 0
[[vk::binding( 0, 0 )]]
ConstantBuffer<Camera> camera;

// descriptor 0 set 1
[[vk::binding( 0, 1 )]]
Sampler2D bindlessTextures[];

// descriptor 0 set 2
[[vk::binding( 0, 2 )]]
StructuredBuffer<MaterialDescription> materials;

[[vk::push_constant]]
PushConstant push;

// VERTEX
[shader( "vertex" )]
VertexOutput vertexMain( VertexInput input )
{
  VertexOutput output;

  output.position =
    mul( camera.projection, mul( camera.view, mul( push.modelMatrix, float4( input.position, 1.0f ) ) ) );

  output.fragTexCoord = input.uv;

  return output;
}

// FRAGMENT
[shader( "fragment" )]
FragmentOutput fragmentMain( VertexOutput input )
{
  FragmentOutput output;

  // If no texture, use a bright magenta to make it obvious
  float4 diffuseColor = float4( 1.0, 0.0, 1.0, 1.0 );

  int diffuseIndex = materials[push.materialIndex].diffuseIndex;
  int specularIndex = materials[push.materialIndex].specularIndex;
  int normalIndex = materials[push.materialIndex].normalIndex;

  if ( diffuseIndex != -1 )
  {
    diffuseColor = bindlessTextures[NonUniformResourceIndex( diffuseIndex )].Sample( input.fragTexCoord );
  }
  else if ( diffuseIndex == -1 && normalIndex == -1 && specularIndex == -1 )
  {
    // use the default material
    diffuseColor = bindlessTextures[NonUniformResourceIndex( 0 )].Sample( input.fragTexCoord );
  }

  output.outColor = diffuseColor;

  return output;
}

