#include "utils.slang"

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

[shader( "fragment" )]
FragmentOutput main( FragmentInput input )
{
  FragmentOutput output;
  output.outColor = push.entityId;
  return output;
}

