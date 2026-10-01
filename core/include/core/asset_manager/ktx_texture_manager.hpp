#pragma once
#include <vulkan/vulkan.h>
#include <ktxvulkan.h>
#include <unordered_map>

namespace resources
{
struct Texture;
}

namespace graphics
{
class VulkanDevice;
class VulkanBuffer;
} // namespace graphics

struct KtxTextureData
{
    int w;
    int h;
    int c;
    uint8_t* pixels;
};

struct TextureLoadData
{
    resources::Texture* texture;
    uint32_t size{ 0u };
    uint32_t offset{ 0u };
    std::filesystem::path path;
};

namespace core
{
class KtxTextureManager
{
  public:
    explicit KtxTextureManager( graphics::VulkanDevice* device );
    ~KtxTextureManager();

    auto loadTexture( const std::filesystem::path p, resources::Texture* texture ) -> KTX_error_code;
    auto saveTexture( const std::filesystem::path p, resources::Texture* texture ) -> KTX_error_code;
    auto saveToKtx( const std::filesystem::path p, KtxTextureData data ) -> KTX_error_code;
    auto destroyTexture( resources::Texture* texture ) -> void;

    auto getDataSize( resources::Texture* texture ) -> uint32_t;
    auto getTextureData( resources::Texture* texture ) -> uint8_t*;

  private:
    ktxVulkanDeviceInfo m_vulkanDeviceInfo;
    graphics::VulkanDevice* m_device;
    ktxVulkanTexture_subAllocatorCallbacks m_subAllocatorCallbacks;
};
} // namespace core