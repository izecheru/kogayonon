#pragma once
#include <filesystem>
#include <glm/glm.hpp>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vulkan/vulkan.h>

struct CreateImageInfo
{
  VkDevice* device;
  VkPhysicalDevice* physicalDevice;
  uint32_t width;
  uint32_t height;
  VkFormat format;
  VkImageTiling tiling;
  VkImageUsageFlags usage;
  VkMemoryPropertyFlags properties;
  VkImage* image;
  VkDeviceMemory* imageMemory;
};

namespace resources
{
class Texture;
class Mesh;
} // namespace resources

struct cgltf_primitive;
struct cgltf_accessor;
struct cgltf_material;

namespace graphics
{
class VulkanDevice;
class VulkanSwapchain;
} // namespace graphics

namespace core
{
class AssetManager
{
public:
  static auto get() -> AssetManager&
  {
    static auto instance = AssetManager{};
    return instance;
  }

  void setSwapchain( graphics::VulkanSwapchain* swapchain );
  void setDevice( graphics::VulkanDevice* device );

  auto getTextureSampler() -> VkSampler&;

  auto addTexture( const std::string& textureName, const std::string& texturePath ) -> resources::Texture*;

  void createImage( const CreateImageInfo& info );
  void transitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout );
  void copyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height );
  auto createImageView( VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags )
    -> VkImageView;
  void createBuffer( VkDeviceSize size,
                     VkBufferUsageFlags usage,
                     VkMemoryPropertyFlags properties,
                     VkBuffer& buffer,
                     VkDeviceMemory& bufferMemory );
  void createTextureSamplers();

protected:
  AssetManager() = default;
  ~AssetManager() = default;

  // copy is not allowed
  AssetManager( const AssetManager& ) = delete;
  AssetManager& operator=( const AssetManager& ) = delete;
  AssetManager( AssetManager&& ) = delete;
  AssetManager& operator=( AssetManager&& ) = delete;

private:
  VkBuffer m_stageBuffer;
  VkDeviceMemory m_stageBufferMemory;

  std::unordered_map<std::string, std::shared_ptr<resources::Texture>> m_loadedTextures;
  std::unordered_map<std::string, std::shared_ptr<resources::Mesh>> m_loadedMeshes;

  graphics::VulkanDevice* m_pDevice;
  graphics::VulkanSwapchain* m_pSwapchain;

  VkSampler m_textureSampler;
};
} // namespace core