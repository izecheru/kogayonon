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

struct PushConstant
{
  float4x4 modelMatrix;
  int materialIndex;
};

[[vk::binding( 0, 0 )]]
ConstantBuffer<Camera> camera;

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
void fragmentMain( VertexOutput input )
{
}

