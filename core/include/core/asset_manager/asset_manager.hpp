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
  uint32_t width;
  uint32_t height;
  VkFormat format;
  VkImageTiling tiling;
  VkImageUsageFlags usage;
  VkMemoryPropertyFlags properties;
  VkImage* image;
  VkDeviceMemory* imageMemory;
};

namespace graphics
{
struct VulkanContext;
}

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

  void setContext( graphics::VulkanContext* ctx );
  auto getTextureSampler() -> VkSampler&;
  auto getTexture( const std::string& texturePath ) -> resources::Texture*;
  auto addTexture( const std::string& textureName, const std::string& texturePath ) -> resources::Texture*;
  void initSampler();

private:
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

  graphics::VulkanContext* m_pVkContext;

  VkSampler m_textureSampler;
};
} // namespace core