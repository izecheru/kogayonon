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
  explicit VulkanTracyContext( VkDevice device,
                               VkPhysicalDevice physicalDevice,
                               VkQueue graphicsQueue,
                               VkCommandPool cmdPool );
  ~VulkanTracyContext();

  auto collect( VkCommandBuffer buffer ) -> void;
  auto getCtx() -> tracy::VkCtx*;
  auto getBuffer() -> VkCommandBuffer;

private:
  tracy::VkCtx* m_tracyContext;
  VkCommandBuffer m_buffer;
};
} // namespace graphics