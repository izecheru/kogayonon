#pragma once

#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
// #define VMA_VULKAN_VERSION 1004000

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>
#include "graphics/vulkan_buffer.hpp"
#include "graphics/vulkan_image.hpp"

namespace graphics
{

class VulkanMemoryAllocator
{
  public:
    explicit VulkanMemoryAllocator( VkDevice device, VkPhysicalDevice physicalDevice, VkInstance instance );
    ~VulkanMemoryAllocator();

    template <typename T>
    inline auto updateBuffer( std::vector<T>& data, VmaAllocation vmaAllocation, VkDeviceSize offset ) -> void;

    template <typename T>
    auto copyFromBuffer( T& data, VulkanBuffer& buffer ) -> void;

    template <typename T>
    auto copyToBuffer( T& data, VulkanBuffer& buffer ) -> void;

    template <typename T>
    auto copyToBuffer( std::vector<T>& data, VulkanBuffer& buffer ) -> void;

    template <typename T>
    auto copyToBuffer( T& data, VulkanBuffer& buffer, VkDeviceSize size ) -> void;

    /**
     * @brief Print the json format to console of allocation data so we know ( if no name was set for that resource )
     * who asserted the deallocation
     * @return
     */
    auto writeLeaksLog() const -> void;

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
    auto setAllocationName( std::string_view name, VmaAllocation& allocation ) const -> void;

    /**
     * @brief Create image
     * @param image VkImage reference
     * @param imageCreateInfo
     * @param usage
     * @param allocation
     * @param imageName Name of the image, "" if not presented
     */
    auto createImage( VkImage& image,
                      VkImageCreateInfo& imageCreateInfo,
                      VmaAllocationCreateInfo& usage,
                      VmaAllocation& allocation,
                      std::string_view imageName = "" ) -> void;

    auto createImage( VulkanImage& image,
                      VkImageCreateInfo& imageCreateInfo,
                      VmaAllocationCreateInfo& usage,
                      std::string_view imageName = "" ) -> void;

    auto createImageView( VkImageView& imageView,
                          VkImage& image,
                          VkFormat format,
                          VkImageAspectFlags aspectFlags,
                          std::string_view imageViewName = "" ) -> VkResult;

    auto invalidateAllocation( VulkanBuffer& vulkanBuffer, VkDeviceSize offset, VkDeviceSize size ) const -> void;

    auto copyMemoryToAllocation( const void* pSrcHostPointer,
                                 VmaAllocation dstAllocation,
                                 VkDeviceSize dstAllocationLocalOffset,
                                 VkDeviceSize size ) const -> VkResult;

    auto getAllocInfo( VmaAllocation allocation ) const -> VmaAllocationInfo;

  private:
    auto formatSize( VkDeviceSize size ) -> std::string;

  private:
    VmaVulkanFunctions m_vulkanFunctions;
    VmaAllocator m_allocator;
    VkInstance m_instance;
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice;
};

#include "graphics/vulkan_memory_allocator.inl"
} // namespace graphics