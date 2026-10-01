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
};

struct Texture
{
    graphics::VulkanImage vulkanImage;
    KtxImage ktxImage;
    uint32_t textureIndex{ 0u };
    uint32_t samplerIndex{ 0u };
    std::string path{ "" };
    std::string name{ "" };
    int width{ 0 };
    int height{ 0 };
    int numComponents{ 0 };
    bool loaded{ false };
};
} // namespace resources