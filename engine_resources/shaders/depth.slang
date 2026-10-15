#include "structures.slang"
#include "utils.slang"
#include "vertex.slang"

struct VertexInput
{
    uint vertexId : SV_VertexID;
    uint instanceId : SV_InstanceID;
    uint baseInstance : SV_StartInstanceLocation;
    uint startVertex : SV_StartVertexLocation;
};

struct EntityParams
{
    float4x4 modelMatrix;
	int materialIndex;
};

struct VertexOutput
{
    float4 position : SV_Position;
    float2 fragTexCoord : TEXCOORD0;
};

struct DrawParams
{
    uint64_t vertexBufferAddress;
    uint64_t entityParamsBufferAddress;
};

DESCRIPTOR_SET( 0, 0, ConstantBuffer<Camera> ) camera;
DECLARE_PUSH_CONSTANT( DrawParams );

[shader( "vertex" )]
VertexOutput vertexMain( VertexInput input )
{
    VertexOutput output;

    Vertex* vBuffer = (Vertex*)pushConstant.vertexBufferAddress;
    EntityParams* entityParams = (EntityParams*)pushConstant.entityParamsBufferAddress;

    Vertex v = vBuffer[input.startVertex + input.vertexId];
    EntityParams p = entityParams[input.baseInstance + input.instanceId];

    output.position = mul( camera.projection, mul( camera.view, mul( p.modelMatrix, float4( v.translation, 1.0f ) ) ) );

    output.fragTexCoord = v.uv;

    return output;
}

[shader( "fragment" )]
void fragmentMain( VertexOutput input )
{
    // only depth
}

