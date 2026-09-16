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
  float4 fragPosLightSpace;
  float3 normal;
};

struct FragmentOutput
{
  float4 outColor : SV_Target0;
};

// Check issue 12 (https://github.com/izecheru/kogayonon/issues/12)
struct MaterialDescription
{
  int normalIndex;
  int diffuseIndex;
  int emissiveIndex;
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

// descriptor 0 set 3
[[vk::binding( 0, 3 )]]
Sampler2D shadowmapTexture;

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
  output.normal = mul( transpose( inverse( float3x3( push.modelMatrix ) ) ), input.normal );

  return output;
}

// FRAGMENT
[shader( "fragment" )]
FragmentOutput fragmentMain( VertexOutput input )
{
  FragmentOutput output;

  float4 diffuseColor = float4( 1.0, 0.0, 1.0, 1.0 );

  int matIdx = push.materialIndex;
  if ( matIdx < 0 )
  {
    diffuseColor = bindlessTextures[NonUniformResourceIndex( 0 )].Sample( input.fragTexCoord );
    output.outColor = diffuseColor;
    return output;
  }

  MaterialDescription mat = materials[matIdx];
  diffuseColor = bindlessTextures[NonUniformResourceIndex( mat.diffuseIndex )].Sample( input.fragTexCoord );
  output.outColor = diffuseColor;

  return output;
}

