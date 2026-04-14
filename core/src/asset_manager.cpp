#include "core/asset_manager/asset_manager.hpp"
#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "graphics/utils.hpp"
#include "graphics/vulkan_context.hpp"
#include "graphics/vulkan_device.hpp"
#include "graphics/vulkan_swapchain.hpp"
#include "resources/mesh.hpp"
#include "resources/texture.hpp"

auto core::AssetManager::addTexture( const std::string& textureName, const std::string& texturePath )
  -> resources::Texture*
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load( texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );

  if ( !pixels )
  {
    spdlog::error( stbi_failure_reason() );
    throw std::runtime_error( "failed to load texture image!" );
  }

  auto texture = std::make_shared<resources::Texture>();
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  createBuffer( m_pVkContext,
                imageSize,
                VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                m_stageBuffer,
                m_stageBufferMemory );

  void* data;
  vkMapMemory( m_pVkContext->device->getLogicalDevice(), m_stageBufferMemory, 0, imageSize, 0, &data );
  memcpy( data, pixels, static_cast<size_t>( imageSize ) );
  vkUnmapMemory( m_pVkContext->device->getLogicalDevice(), m_stageBufferMemory );

  stbi_image_free( pixels );

  CreateImageInfo info{ .width = static_cast<uint32_t>( texWidth ),
                        .height = static_cast<uint32_t>( texHeight ),
                        .format = VK_FORMAT_R8G8B8A8_UNORM,
                        .tiling = VK_IMAGE_TILING_OPTIMAL,
                        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                        .properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                        .image = &texture->getImage(),
                        .imageMemory = &texture->getMemory() };

  createImage( m_pVkContext, info );
  texture->getView() =
    createImageView( m_pVkContext, texture->getImage(), VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT );

  transitionImageLayout( m_pVkContext,
                         texture->getImage(),
                         VK_FORMAT_R8G8B8A8_UNORM,
                         VK_IMAGE_LAYOUT_UNDEFINED,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );

  copyBufferToImage( m_pVkContext,
                     m_stageBuffer,
                     texture->getImage(),
                     static_cast<uint32_t>( texWidth ),
                     static_cast<uint32_t>( texHeight ) );

  transitionImageLayout( m_pVkContext,
                         texture->getImage(),
                         VK_FORMAT_R8G8B8A8_UNORM,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );

  vkDestroyBuffer( m_pVkContext->device->getLogicalDevice(), m_stageBuffer, nullptr );
  vkFreeMemory( m_pVkContext->device->getLogicalDevice(), m_stageBufferMemory, nullptr );

  m_loadedTextures.emplace( texturePath, std::move( texture ) );

  return m_loadedTextures.at( texturePath ).get();
}

auto core::AssetManager::getTextureSampler() -> VkSampler&
{
  return m_textureSampler;
}

void core::AssetManager::setContext( graphics::VulkanContext* ctx )
{
  m_pVkContext = ctx;
}

auto core::AssetManager::getTexture( const std::string& texturePath ) -> resources::Texture*
{
  return m_loadedTextures.at( texturePath ).get();
}

void core::AssetManager::initSampler()
{
  createSampler(
    m_pVkContext->device->getLogicalDevice(), m_pVkContext->device->getPhysicalDevice(), m_textureSampler );
}
