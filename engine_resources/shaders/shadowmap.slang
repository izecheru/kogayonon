
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

struct PushConstant
{
  float4x4 modelMatrix;
  int materialIndex;
};

// descriptor 0 set 0
[[vk::binding( 0, 0 )]] // this is the directional light POV
ConstantBuffer<Camera> camera;

[[vk::binding( 0, 1 )]]
Texture2D directionalLightTexture;

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

