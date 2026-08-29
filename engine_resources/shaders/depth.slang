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

// descriptor 0 set 2
[[vk::binding( 0, 1 )]]
Sampler2D bindlessTextures[];

// descriptor 0 set 3
[[vk::binding( 0, 2 )]]
StructuredBuffer<MaterialDescription, ScalarDataLayout> materials;

[[vk::push_constant]]
PushConstant push;

MaterialDescription getMaterial( int materialIndex )
{
  return materials[materialIndex];
}

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
  // depth is automatically written here
  FragmentOutput output;
  output.outColor = float4( 0, 0, 0, 0 );
  return output;
}

