#include "structures.slang"

struct VertexInput
{
  float3 position : POSITION;
  float3 normal : NORMAL;
  float2 uv : TEXCOORD0;
};

struct VertexOutput
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
VertexOutput vertexMain( VertexInput vertexInput )
{
  VertexOutput vertexOutput;

  vertexOutput.position =
    mul( camera.projection, mul( camera.view, mul( push.modelMatrix, float4( vertexInput.position, 1.0f ) ) ) );

  return vertexOutput;
}

[shader( "fragment" )]
FragmentOutput fragmentMain( VertexOutput input )
{
  FragmentOutput output;
  output.outColor = push.entityId;
  return output;
}

