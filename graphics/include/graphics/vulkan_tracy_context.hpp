#pragma once
#include <vulkan/vulkan.h>

namespace tracy
{
class VkCtx;
}

namespace graphics
{
class VulkanTracyContext
{
public:
  VulkanTracyContext() = default;
  ~VulkanTracyContext();

  auto collect( VkCommandBuffer buffer ) -> void;
  auto getCtx() -> tracy::VkCtx*;
  auto getBuffer() -> VkCommandBuffer;
  auto initCtx( VkDevice device, VkPhysicalDevice physicalDevice, VkQueue graphicsQueue, VkCommandPool cmdPool )
    -> void;

private:
  tracy::VkCtx* m_tracyContext;
  VkCommandBuffer m_buffer;
};
} // namespace graphics