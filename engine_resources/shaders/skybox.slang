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
    uint32_t cubemapIndex;
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
    // +Z face (front): 0,1,2  2,3,0
    0,
    1,
    2,
    2,
    3,
    0,
    // -Z face (back): 4,5,6  6,7,4
    4,
    5,
    6,
    6,
    7,
    4,
    // +X face (right): 1,5,6  6,2,1
    1,
    5,
    6,
    6,
    2,
    1,
    // -X face (left): 0,4,7  7,3,0
    0,
    4,
    7,
    7,
    3,
    0,
    // +Y face (top): 3,2,6  6,7,3
    3,
    2,
    6,
    6,
    7,
    3,
    // -Y face (bottom): 0,1,5  5,4,0
    0,
    1,
    5,
    5,
    4,
    0 );

[shader( "vertex" )]
VertexOutput vertexMain( VertexInput input )
{
    VertexOutput o;

    int index = indices[input.vertexID];
    float3 localPos = pos[index];

    float4x4 viewMat = camera.view;
    viewMat[0][3] = 0.0;
    viewMat[1][3] = 0.0;
    viewMat[2][3] = 0.0;

    float4 outPosition = mul( camera.projection, mul( viewMat, float4( localPos, 1.0 ) ) );
    o.position = outPosition.xyww;
    o.viewDir = normalize(localPos);

    return o;
}

[shader( "fragment" )]
FragmentOutput fragmentMain( VertexOutput input )
{
    FragmentOutput output;
    output.outColor = bindlessCubemaps[NonUniformResourceIndex( pushConstant.cubemapIndex )].Sample( input.viewDir );

    return output;
}

