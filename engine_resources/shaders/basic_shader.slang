#include "structures.slang"
#include "utils.slang"
#include "material.slang"
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
    float3 worldNormal : TEXCOORD1;
    float4 fragPosLightSpace : TEXCOORD2;
    uint baseInstance : TEXCOORD3;
};

struct FragmentOutput
{
    float4 outColor : SV_Target0;
};

struct DrawParams
{
    uint64_t vertexBufferAddress;
    uint64_t materialBufferAddress;
    uint64_t entityParamsBufferAddress;
};

DESCRIPTOR_SET( 0, 0, ConstantBuffer<Camera> ) camera;
DESCRIPTOR_SET( 0, 1, Sampler2D[] ) bindlessTextures;
DECLARE_PUSH_CONSTANT( DrawParams );

[shader( "vertex" )]
VertexOutput vertexMain( VertexInput input )
{
    VertexOutput output;

    Vertex* vBuffer = (Vertex*)pushConstant.vertexBufferAddress;
    EntityParams* entityParams = (EntityParams*)pushConstant.entityParamsBufferAddress;

    Vertex v = vBuffer[input.vertexId + input.startVertex];
    EntityParams p = entityParams[input.baseInstance + input.instanceId];

    float4 worldPos = mul( p.modelMatrix, float4( v.translation, 1.0f ) );
    output.position = mul( camera.projection, mul( camera.view, worldPos ) );

    output.fragTexCoord = v.uv;
    output.baseInstance = input.baseInstance + input.instanceId;
    output.fragPosLightSpace = float4( 0, 0, 0, 1 );

    return output;
}

[shader( "fragment" )]
FragmentOutput fragmentMain( VertexOutput input )
{
    FragmentOutput output;

    EntityParams* entityParams = (EntityParams*)pushConstant.entityParamsBufferAddress;
    EntityParams p = entityParams[input.baseInstance];

    if ( p.materialIndex < 0 )
    {
        output.outColor = float4( 0.0, 0.0, 0.0, 1.0 );
        return output;
    }

    Material* materialBuffer = (Material*)pushConstant.materialBufferAddress;
    Material mat = materialBuffer[p.materialIndex];

    float4 diffuseColor = bindlessTextures[NonUniformResourceIndex( mat.diffuseIndex )].Sample( input.fragTexCoord );

    output.outColor = diffuseColor;
    return output;
}

