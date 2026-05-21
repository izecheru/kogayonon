#include "structures.slang"

struct VertexInput
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
};

struct VertexOut
{
  float4 position : SV_Position;
};

struct FragmentInput
{
  float4 position : SV_Position;
};

struct FragmentOutput
{
  int outColor : SV_Target0;
};

struct PushConstant
{
  float4x4 modelMatrix;
  int entityId;
};

[[vk::push_constant]]
PushConstant push;

[[vk::binding( 0, 0 )]]
ConstantBuffer<Camera> camera;

[shader( "vertex" )]
VertexOut vertexMain( VertexInput vertexInput )
{
  VertexOut vertexOutput;

  float4 pos = float4( vertexInput.position, 1.0 );
  pos = mul( push.modelMatrix, pos );
  pos = mul( camera.view, pos );
  pos = mul( camera.projection, pos );

  vertexOutput.position = pos;

  return vertexOutput;
}

[shader( "fragment" )]
FragmentOutput fragmentMain( FragmentInput input )
{
  FragmentOutput output;
  output.outColor = push.entityId;
  return output;
}

