#pragma once
#include <glm/glm.hpp>

namespace resources
{

struct FramePushConstant
{
    VkDeviceAddress vertexBufferAddress{ 0u };
    VkDeviceAddress entityParamsBufferAddress{ 0u };
};

struct MeshPushConstant
{
    glm::mat4 modelMatrix;
    int materialIndex;
};

static_assert( sizeof( MeshPushConstant ) <= 128, "Vulkan push constants must be lower than 128 bytes" );

struct EntityPickingPushConstant
{
    glm::mat4 modelMatrix;
    int entityId{ -1 };
};

static_assert( sizeof( MeshPushConstant ) <= 128, "Vulkan push constants must be lower than 128 bytes" );
} // namespace resources