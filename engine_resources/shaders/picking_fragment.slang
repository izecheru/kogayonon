#include "utils.slang"

struct FragmentInput
{
  float4 position : SV_Position;
};

struct FragmentOutput
{
  uint32_t outColor : SV_Target0;
};

struct PushConstant
{
  float4x4 modelMatrix;
  uint32_t entityId;
};

[[vk::push_constant]]
PushConstant push;

[shader( "fragment" )]
FragmentOutput main( FragmentInput input )
{
  FragmentOutput output;
  float4 colorResult = colorFromNumber( push.entityId );
  output.outColor = push.entityId;
  return output;
}

