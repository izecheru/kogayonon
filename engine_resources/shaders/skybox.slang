#include "structures.slang"
#include "inverse.slang"
#include "utils.slang"
#include "material.slang"
#include "vertex.slang"

struct VertexInput
{
    uint vertexID : SV_VertexID;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float3 viewDir : TEXCOORD0;
};

struct FragmentOutput
{
    float4 outColor : SV_Target0;
};

struct DrawParams
{
    int cubemapIndex;
};

DESCRIPTOR_SET( 0, 0, ConstantBuffer<Camera> ) camera;
DESCRIPTOR_SET( 0, 1, SamplerCube[] ) bindlessCubemaps;
DECLARE_PUSH_CONSTANT( DrawParams );

static float3 pos[8] = float3[8]( float3( -1.0, -1.0, 1.0 ),
                                  float3( 1.0, -1.0, 1.0 ),
                                  float3( 1.0, 1.0, 1.0 ),
                                  float3( -1.0, 1.0, 1.0 ),

                                  float3( -1.0, -1.0, -1.0 ),
                                  float3( 1.0, -1.0, -1.0 ),
                                  float3( 1.0, 1.0, -1.0 ),
                                  float3( -1.0, 1.0, -1.0 )

);

static int indices[36] = int[36](
    1, 0, 2, 3, 2, 0, 5, 1, 6, 2, 6, 1, 6, 7, 5, 4, 5, 7, 0, 4, 3, 7, 3, 4, 5, 4, 1, 0, 1, 4, 2, 3, 6, 7, 6, 3 );

[shader( "vertex" )]
VertexOutput vertexMain( VertexInput input )
{
    VertexOutput o;

    int index = indices[input.vertexID];
    float3 localPos = pos[index];

    float4 viewPos = mul( camera.view, float4( localPos, 0.0 ) );
    float4 clip = mul( camera.projection, float4( viewPos.xyz, 1.0 ) );

    clip.z = clip.w;
    o.position = clip;
    o.viewDir = localPos;

    return o;
}

[shader( "fragment" )]
FragmentOutput fragmentMain( VertexOutput input )
{
    FragmentOutput output;
    float3 dir = normalize( input.viewDir );
    output.outColor = bindlessCubemaps[NonUniformResourceIndex( pushConstant.cubemapIndex )].Sample( dir );
    return output;
}

