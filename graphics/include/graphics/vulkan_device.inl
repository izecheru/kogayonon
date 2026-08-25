#pragma once
#include "graphics/vulkan_device.hpp"

template <typename T>
inline auto graphics::VulkanDevice::vulkanTypeToObject() const -> VkObjectType
{
  if constexpr ( std::is_same_v<T, VkImage> )
    return VK_OBJECT_TYPE_IMAGE;
  else if constexpr ( std::is_same_v<T, VkBuffer> )
    return VK_OBJECT_TYPE_BUFFER;
  else if constexpr ( std::is_same_v<T, VkSampler> )
    return VK_OBJECT_TYPE_SAMPLER;
  else if constexpr ( std::is_same_v<T, VkInstance> )
    return VK_OBJECT_TYPE_INSTANCE;
  else if constexpr ( std::is_same_v<T, VkCommandBuffer> )
    return VK_OBJECT_TYPE_COMMAND_BUFFER;
  else if constexpr ( std::is_same_v<T, VkPhysicalDevice> )
    return VK_OBJECT_TYPE_PHYSICAL_DEVICE;
  else if constexpr ( std::is_same_v<T, VkDevice> )
    return VK_OBJECT_TYPE_DEVICE;
  else if constexpr ( std::is_same_v<T, VkQueue> )
    return VK_OBJECT_TYPE_QUEUE;
  else if constexpr ( std::is_same_v<T, VkSemaphore> )
    return VK_OBJECT_TYPE_SEMAPHORE;
  else if constexpr ( std::is_same_v<T, VkFence> )
    return VK_OBJECT_TYPE_FENCE;
  else if constexpr ( std::is_same_v<T, VkDeviceMemory> )
    return VK_OBJECT_TYPE_DEVICE_MEMORY;
  else if constexpr ( std::is_same_v<T, VkEvent> )
    return VK_OBJECT_TYPE_EVENT;
  else if constexpr ( std::is_same_v<T, VkQueryPool> )
    return VK_OBJECT_TYPE_QUERY_POOL;
  else if constexpr ( std::is_same_v<T, VkBufferView> )
    return VK_OBJECT_TYPE_BUFFER_VIEW;
  else if constexpr ( std::is_same_v<T, VkImageView> )
    return VK_OBJECT_TYPE_IMAGE_VIEW;
  else if constexpr ( std::is_same_v<T, VkShaderModule> )
    return VK_OBJECT_TYPE_SHADER_MODULE;
  else if constexpr ( std::is_same_v<T, VkPipelineCache> )
    return VK_OBJECT_TYPE_PIPELINE_CACHE;
  else if constexpr ( std::is_same_v<T, VkPipelineLayout> )
    return VK_OBJECT_TYPE_PIPELINE_LAYOUT;
  else if constexpr ( std::is_same_v<T, VkRenderPass> )
    return VK_OBJECT_TYPE_RENDER_PASS;
  else if constexpr ( std::is_same_v<T, VkPipeline> )
    return VK_OBJECT_TYPE_PIPELINE;
  else if constexpr ( std::is_same_v<T, VkDescriptorSetLayout> )
    return VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT;
  else if constexpr ( std::is_same_v<T, VkDescriptorPool> )
    return VK_OBJECT_TYPE_DESCRIPTOR_POOL;
  else if constexpr ( std::is_same_v<T, VkDescriptorSet> )
    return VK_OBJECT_TYPE_DESCRIPTOR_SET;
  else if constexpr ( std::is_same_v<T, VkFramebuffer> )
    return VK_OBJECT_TYPE_FRAMEBUFFER;
  else if constexpr ( std::is_same_v<T, VkCommandPool> )
    return VK_OBJECT_TYPE_COMMAND_POOL;
  else if constexpr ( std::is_same_v<T, VkDescriptorUpdateTemplate> )
    return VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE;
  else if constexpr ( std::is_same_v<T, VkSamplerYcbcrConversion> )
    return VK_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION;
  else if constexpr ( std::is_same_v<T, VkPrivateDataSlot> )
    return VK_OBJECT_TYPE_PRIVATE_DATA_SLOT;
  else if constexpr ( std::is_same_v<T, VkSurfaceKHR> )
    return VK_OBJECT_TYPE_SURFACE_KHR;
  else if constexpr ( std::is_same_v<T, VkSwapchainKHR> )
    return VK_OBJECT_TYPE_SWAPCHAIN_KHR;
  else if constexpr ( std::is_same_v<T, VkDisplayKHR> )
    return VK_OBJECT_TYPE_DISPLAY_KHR;
  else if constexpr ( std::is_same_v<T, VkDisplayModeKHR> )
    return VK_OBJECT_TYPE_DISPLAY_MODE_KHR;
  else if constexpr ( std::is_same_v<T, VkDebugReportCallbackEXT> )
    return VK_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT;
  else if constexpr ( std::is_same_v<T, VkVideoSessionKHR> )
    return VK_OBJECT_TYPE_VIDEO_SESSION_KHR;
  else if constexpr ( std::is_same_v<T, VkVideoSessionParametersKHR> )
    return VK_OBJECT_TYPE_VIDEO_SESSION_PARAMETERS_KHR;
  else if constexpr ( std::is_same_v<T, VkDebugUtilsMessengerEXT> )
    return VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT;
  else if constexpr ( std::is_same_v<T, VkAccelerationStructureKHR> )
    return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR;
  else if constexpr ( std::is_same_v<T, VkValidationCacheEXT> )
    return VK_OBJECT_TYPE_VALIDATION_CACHE_EXT;
  else if constexpr ( std::is_same_v<T, VkAccelerationStructureNV> )
    return VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV;
  else if constexpr ( std::is_same_v<T, VkPerformanceConfigurationINTEL> )
    return VK_OBJECT_TYPE_PERFORMANCE_CONFIGURATION_INTEL;
  else if constexpr ( std::is_same_v<T, VkDeferredOperationKHR> )
    return VK_OBJECT_TYPE_DEFERRED_OPERATION_KHR;
  else if constexpr ( std::is_same_v<T, VkIndirectCommandsLayoutNV> )
    return VK_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NV;
  else if constexpr ( std::is_same_v<T, VkMicromapEXT> )
    return VK_OBJECT_TYPE_MICROMAP_EXT;
  else if constexpr ( std::is_same_v<T, VkOpticalFlowSessionNV> )
    return VK_OBJECT_TYPE_OPTICAL_FLOW_SESSION_NV;
  else
    return VK_OBJECT_TYPE_UNKNOWN;
}

template <typename T>
inline auto graphics::VulkanDevice::updateBuffer( std::vector<T>& vectorData,
                                                  VmaAllocation vmaAllocation,
                                                  VkDeviceSize offset ) -> void
{
  vmaCopyMemoryToAllocation( m_allocator, vectorData.data(), vmaAllocation, offset, sizeof( T ) * vectorData.size() );
}

template <typename T>
inline auto graphics::VulkanDevice::copyBufferData( T& data, VulkanBuffer& buffer, VkDeviceSize size ) -> void
{
  std::memcpy( &data, buffer.mappedData, size );
}

template <typename T>
inline auto graphics::VulkanDevice::copyDataToBuffer( T& data, VulkanBuffer& buffer, VkDeviceSize size ) -> void
{
  std::memcpy( buffer.mappedData, &data, size );
}
