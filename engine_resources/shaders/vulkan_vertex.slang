#version 460

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

struct Camera
{
  float4x4 proj;
  float4x4 view;
};

[[vk::binding( 0, 0 )]]
ConstantBuffer<Camera> camera;

struct PushConstant
{
  float4x4 modelMatrix;
  int materialIndex;
};

// push constant
[[vk::push_constant]]
PushConstant push;

[shader( "vertex" )]
VertexOut main( VertexInput vertexInput )
{
  VertexOut vertexOutput;

  float4 pos = float4( vertexInput.position, 1.0 );
  pos = mul( push.modelMatrix, pos );
  pos = mul( camera.proj, pos );
  pos = mul( camera.view, pos );

  vertexOutput.position = pos;
  vertexOutput.fragTexCoord = vertexInput.uv;

  return vertexOutput;
}

