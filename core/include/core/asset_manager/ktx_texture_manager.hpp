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
} // namespace graphics

struct KtxTextureData
{
    int w;
    int h;
    int c;
    unsigned char* pixels;
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
    auto convertToKtx( const std::filesystem::path p, KtxTextureData data ) -> KTX_error_code;
    auto destroyTexture( resources::Texture* texture ) -> void;

  private:
    ktxVulkanDeviceInfo m_vulkanDeviceInfo;
    graphics::VulkanDevice* m_device;
};
} // namespace core