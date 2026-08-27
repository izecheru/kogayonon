#pragma once

#include "graphics/vulkan_buffer.hpp"
#include "precompiled/pch.hpp"
#include <vulkan/vulkan.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
// #define VMA_VULKAN_VERSION 1004000
#include <vma/vk_mem_alloc.h>

struct SDL_Window;

namespace graphics
{

inline const std::vector<const char*> validationLayers{ "VK_LAYER_KHRONOS_validation" };
inline const std::vector<const char*> deviceExtensions{
  VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME };

#ifdef _DEBUG
const bool enableValidationLayers{ true };
#else
const bool enableValidationLayers{ false };
#endif

struct VulkanPlatform
{
  VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
  VkSurfaceKHR surface{ VK_NULL_HANDLE };
  VkInstance instance{ VK_NULL_HANDLE };
  VkDevice device{ VK_NULL_HANDLE };
};

struct QueueFamilyIndices
{
  std::optional<uint32_t> graphicsFamily;
  std::optional<uint32_t> presentFamily;
  std::optional<uint32_t> transferFamily;

  bool isComplete() const
  {
    return graphicsFamily.has_value() && presentFamily.has_value() && transferFamily.has_value();
  }
};

struct GPUQueue
{
  VkQueue handle{ VK_NULL_HANDLE };
  uint32_t familyIndex = UINT32_MAX;
};

class ShaderCompiler;
struct VulkanBuffer;
struct VulkanImage;
struct FrameInFlightVulkanBuffer;
struct ImageTransitionData;

class VulkanDevice
{
public:
  explicit VulkanDevice( SDL_Window* wnd );
  ~VulkanDevice();

  /**
   * @brief Print the json format to console of allocation data so we know ( if no name was set for that resource )
   * who asserted the deallocation
   * @return
   */
  auto printLeaks() const -> void;

  /**
   * @brief Create vulkan buffer
   * @param vulkanBuffer
   * @param createInfo
   * @param usage
   * @param bufferName
   */
  auto createBuffer( VulkanBuffer& vulkanBuffer,
                     VkBufferCreateInfo& createInfo,
                     VmaAllocationCreateInfo& usage,
                     std::string_view bufferName = "" ) -> void;

  /**
   * @brief Create frame in flight buffer
   * @param vulkanBuffer
   * @param createInfo
   * @param usage
   */
  auto createBuffer( FrameInFlightVulkanBuffer& vulkanBuffer,
                     VkBufferCreateInfo& createInfo,
                     VmaAllocationCreateInfo& usage,
                     std::string_view name ) -> void;

  /**
   * @brief Create staging buffer, used primarely for small operations like copying data into other buffers
   * @param createInfo
   * @param usage
   * @return
   */
  auto createStagingBuffer( VkBufferCreateInfo& createInfo, VmaAllocationCreateInfo& usage ) -> VulkanBuffer;

  template <typename T>
  auto copyBufferData( T& data, VulkanBuffer& buffer, VkDeviceSize size ) -> void;

  template <typename T>
  auto copyDataToBuffer( T& data, VulkanBuffer& buffer, VkDeviceSize size ) -> void;

  template <typename T>
  auto copyDataToBuffer( const std::vector<T> data, VulkanBuffer& buffer, VkDeviceSize size ) -> void;

  auto invalidateAllocation( VulkanBuffer& vulkanBuffer, VkDeviceSize offset, VkDeviceSize size ) const -> void;

  /**
   * @brief Create image
   * @param image VkImage reference
   * @param imageCreateInfo
   * @param usage
   * @param allocation
   * @param imageName Name of the image, "" if not presented
   */
  void createImage( VkImage& image,
                    VkImageCreateInfo& imageCreateInfo,
                    VmaAllocationCreateInfo& usage,
                    VmaAllocation& allocation,
                    std::string_view imageName = "" );

  void createImage( VulkanImage& image,
                    VkImageCreateInfo& imageCreateInfo,
                    VmaAllocationCreateInfo& usage,
                    std::string_view imageName = "" );

  auto createImageView( VkImageView& imageView, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags ) const
    -> void;
  auto createImageView( VulkanImage& image, VkFormat format, VkImageAspectFlags aspectFlags ) const -> void;

  /**
   * @brief Transition image layout with a newly allocated one-time-use VkCommandBuffer
   * @param image
   * @param transitionData
   * @return
   */
  auto transitionImageLayout( VkImage image, ImageTransitionData transitionData ) -> void;
  auto transitionImageLayout( VulkanImage image, ImageTransitionData transitionData ) -> void;

  /**
   * @brief Transition image layout with the command buffer from swapchain instead of a
   * single use new command buffer
   * @param image
   * @param cmdBuffer
   * @param transitionData
   * @return
   */
  auto transitionImageLayout( VkImage image, VkCommandBuffer cmdBuffer, ImageTransitionData transitionData ) -> void;
  auto transitionImageLayout( VulkanImage image, VkCommandBuffer cmdBuffer, ImageTransitionData transitionData )
    -> void;

  auto createSampler( VkSampler& sampler ) const -> void;
  auto copyBuffer( VkBuffer src, VkBuffer dst, VkDeviceSize size ) const -> void;
  auto copyBufferToImage(
    VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, ImageTransitionData transitionData ) -> void;
  auto copyImageToBuffer( VkImage image, VkBuffer buffer, VkOffset3D offset, VkExtent3D extent ) -> void;
  auto copyImageToBuffer( VkImage image,
                          VkCommandBuffer cmdBuffer,
                          VkBuffer buffer,
                          VkOffset3D offset,
                          VkExtent3D extent,
                          bool applyBarrier = true ) -> void;

  auto endSingleTimeCommands( VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool ) const -> void;

  auto endSingleTimeCommands( VkCommandBuffer commandBuffer,
                              VkQueue queue,
                              VkSubmitInfo submitInfo,
                              VkCommandPool pool ) const -> void;

  auto beginSingleTimeCommands( VkCommandPool pool ) const -> VkCommandBuffer;
  auto beginSingleTimeCommands() const -> VkCommandBuffer;

  auto destroyDescriptorSetLayout( VkDescriptorSetLayout layout ) const -> void;
  auto destroyDescriptorPool( VkDescriptorPool pool ) const -> void;

  auto getCommandPool() -> VkCommandPool;
  auto getTransferCommandPool() -> VkCommandPool;

  /**
   * @brief Deallocate image and destroy it
   * @param image
   * @param allocation
   * @return
   */
  auto destroyImage( VkImage& image, VmaAllocation& allocation ) -> void;

  /**
   * @brief Deallocate images and destroy them
   * @param images
   * @return
   */
  auto destroyImages( const std::initializer_list<std::tuple<VkImage&, VmaAllocation&>>& images ) -> void;

  /**
   * @brief Destroy buffer
   * @param buff
   * @return
   */
  auto destroyBuffer( VulkanBuffer& buff ) -> void;

  /**
   * @brief Destroy frame in flight buffer
   * @param buff
   * @return
   */
  auto destroyBuffer( FrameInFlightVulkanBuffer& buff ) -> void;

  /**
   * @brief VmaAllocator getter
   * @return
   */
  auto getAllocator() -> VmaAllocator&;

  /**
   * @brief Set name for the VmaAllocation
   * @param name
   * @param allocation
   * @return
   */
  auto setName( std::string_view name, VmaAllocation& allocation ) const -> void;

  auto createDescriptorSetLayout( VkDescriptorSetLayout& layout, VkDescriptorSetLayoutCreateInfo& layoutInfo ) const
    -> void;
  auto createDescriptorSet() -> void;
  auto allocateDescriptorSet( VkDescriptorSet& descriptor, VkDescriptorSetAllocateInfo& info ) const -> void;
  auto updateDescriptorSet( std::initializer_list<VkWriteDescriptorSet> writes,
                            uint32_t writeCount = 1,
                            uint32_t copyCount = 0 ) const -> void;

  //	vmaCopyMemoryToAllocation( m_vkCtx->device->getAllocator(),
  // m_materials.data(),
  // m_materialsBuffer.allocation,
  // 0,
  // sizeof( resources::Material ) * m_materials.size() );

  template <typename T>
  auto updateBuffer( std::vector<T>& vectorData, VmaAllocation allocation, VkDeviceSize offset = 0 ) -> void;

  /**
   * @brief Destroy the shader module
   * @param module
   * @return
   */
  auto destroyShaderModule( VkShaderModule module ) const -> void;
  auto destroyShaderModule( std::initializer_list<VkShaderModule> modules ) const -> void;

  /**
   * @brief Destroy the image view
   * @param imageView
   * @return
   */
  auto destroyImageView( VkImageView imageView ) const -> void;
  auto destroyImageViews( std::initializer_list<VkImageView> imageViews ) const -> void;
  auto destroySampler( VkSampler sampler ) const -> void;
  auto destroyPipeline( VkPipeline pipeline ) const -> void;
  auto destroyPipelineLayout( VkPipelineLayout layout ) const -> void;
  auto destroyCommandPool( VkCommandPool pool ) const -> void;
  auto destroySemaphore( VkSemaphore semaphore ) const -> void;
  auto destroyFence( VkFence fence ) const -> void;

  auto getSwapchainImagesKHR( std::vector<VulkanImage>& swapchainImages,
                              VkSwapchainKHR swapchain,
                              uint32_t& imageCount ) const -> void;
  auto createSwapchain( VkSwapchainCreateInfoKHR info, VkSwapchainKHR* swapchain ) const -> void;
  auto destroySwapchain( VkSwapchainKHR swapchain ) const -> void;

  /**
   * @brief Initialize the vulkan device
   * @return
   */
  auto init() -> void;
  auto createCommandPool( VkCommandPoolCreateInfo createInfo, VkCommandPool& pool ) const -> void;
  auto createPipelineLayout( VkPipelineLayoutCreateInfo layout, VkPipelineLayout& pipelineLayout ) const -> void;
  auto createGraphicsPipeline( VkPipeline& pipeline, VkGraphicsPipelineCreateInfo createInfo ) const -> void;

  auto initMemoryAllocator() -> void;

  /**
   * @brief Setup debug function pointers for vulkan validation layers
   * @return
   */
  auto setupDebug() -> void;

  /**
   * @brief Create the instance
   * @return
   */
  auto createInstance() -> void;

  /**
   * @brief Create window surface
   * @return
   */
  auto createWindowSurface() -> void;

  /**
   * @brief Iterate through available GPUs and find one suitable for operations we want to perform
   * @return
   */
  auto pickPhysicalDevice() -> void;

  /**
   * @brief Create the logical device
   * @return
   */
  auto createLogicalDevice() -> void;

  /**
   * @brief Perform the shutdown operations
   * @return
   */
  auto shutdown() const -> void;

  /**
   * @brief Wait for the GPU to finish all commands and enter idle state
   * @return
   */
  auto waitIdle() const -> void;

  /**
   * @brief Set the name of a vulkan object for debugging, if a validation layer error appears,
   * the name will be printed too
   * @param objType Type of vulkan object
   * @param handle Handle of the object
   * @param name Name we want to assign to that object
   * @return
   */
  auto setDebugName( VkObjectType objType, uint64_t handle, std::string_view name ) -> void;

  /**
   * @brief Return the graphics queue
   * @return
   */
  auto getGraphicsQueue() -> GPUQueue&;

  /**
   * @brief Return the present queue
   * @return
   */
  auto getPresentQueue() -> GPUQueue&;

  auto getTransferQueue() -> GPUQueue&;

  /**
   * @brief Get the physical device
   * @return
   */
  auto getPhysicalDevice() const -> VkPhysicalDevice;

  /**
   * @brief Get the logical device
   * @return
   */
  auto getLogicalDevice() const -> VkDevice;

  /**
   * @brief Get the surface
   * @return
   */
  auto getSurface() const -> VkSurfaceKHR;

  /**
   * @brief Get the instance
   * @return
   */
  auto getInstance() const -> VkInstance;

  /**
   * @brief Get physical device limits
   * @return
   */
  auto getLimits() -> VkPhysicalDeviceLimits&;

  auto createFence( VkFence& fence, VkFenceCreateInfo info ) const -> void;

  auto createTimelineSemaphore( VkSemaphore& timeline,
                                VkSemaphoreTypeCreateInfo info,
                                VkSemaphoreCreateInfo createInfo ) const -> void;

  auto createShaderModule( const std::string& shaderName, const std::string& shaderEntryFunc ) -> VkShaderModule;
  auto destroyShaderModule( VkShaderModule module ) -> void;

  auto findSupportedFormat( VkPhysicalDevice device,
                            const std::vector<VkFormat>& candidates,
                            VkImageTiling tiling,
                            VkFormatFeatureFlags features ) -> VkFormat;
  auto findDepthFormat( VkPhysicalDevice device ) -> VkFormat;

  auto getDeviceProperties() const -> VkPhysicalDeviceProperties;
  auto getDeviceMemoryProperties() const -> VkPhysicalDeviceMemoryProperties;

private:
  auto createDeviceCommandPool() -> void;
  auto checkDeviceExtensionSupport( VkPhysicalDevice device ) -> bool;
  auto checkValidationLayerSupport() -> bool;

  template <typename T>
  auto vulkanTypeToObject() const -> VkObjectType;

  auto formatSize( VkDeviceSize size ) -> std::string;
  auto getAllocInfo( VmaAllocation allocation ) const -> VmaAllocationInfo;

  auto findQueueFamilies( VkPhysicalDevice& device ) -> QueueFamilyIndices;
  bool isDeviceSuitable( VkPhysicalDevice& device );
  auto getRequiredExtensions() -> std::vector<const char*>;

private:
  SDL_Window* m_window;
  VulkanPlatform m_platform;

  VkPhysicalDeviceProperties m_physicalDeviceProps;
  VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProps;
  VkPhysicalDeviceFeatures m_physicalDeviceFeatures;
  VkPhysicalDeviceLimits m_physicalDeviceLimits;

  VkCommandPool m_commandPool;
  VkCommandPool m_transferCommandPool;

  VkDebugUtilsMessengerEXT m_debugMessenger{ VK_NULL_HANDLE };

  GPUQueue m_presentQueue;
  GPUQueue m_graphicsQueue;
  GPUQueue m_transferQueue;

  PFN_vkSetDebugUtilsObjectNameEXT m_setDebugNameFunc{ nullptr };
  VmaAllocator m_allocator;
  std::unique_ptr<ShaderCompiler> m_shaderCompiler;
};

#include "vulkan_device.inl"
} // namespace graphics
