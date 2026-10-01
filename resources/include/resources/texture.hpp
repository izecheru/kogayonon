#pragma once
#include <vulkan/vulkan.h>
#include <ktxvulkan.h>
#include "graphics/vulkan_image.hpp"

namespace resources
{
struct KtxImage
{
    ktxTexture2* texture;
    ktxVulkanTexture vulkanTexture;
    bool uploaded{ false };

    /**
     * @brief This is to retrieve the normal texture from the asset manager map
     * to update the texture if needed
     */
    std::filesystem::path normalTexturePath{};
};

struct Texture
{
    graphics::VulkanImage image;
    KtxImage ktxImage;
    uint32_t textureIndex{ 0u };
    uint32_t samplerIndex{ 0u };
    std::string path{ "" };
    std::string name{ "" };
    int width{ 0 };
    int height{ 0 };
    int numComponents{ 0 };
    bool loaded{ false };
    bool isKtx{ false };
};
} // namespace resources