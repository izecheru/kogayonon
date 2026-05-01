#pragma once
#include <glm/glm.hpp>

namespace resources
{

struct MeshPushConstant
{
  glm::mat4 modelMatrix;
  int materialIndex;
};

// NOTE you can actually check if the device supports more than 128
static_assert( sizeof( MeshPushConstant ) < 128, "Vulkan push constants must be lower than 128 bytes" );
} // namespace resources