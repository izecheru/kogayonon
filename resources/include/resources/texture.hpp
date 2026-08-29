#pragma once
#include "graphics/vulkan_image.hpp"
#include "precompiled/pch.hpp"
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace resources
{
class Texture
{
public:
  Texture() = default;

  // without index
  explicit Texture( const std::string& p, const std::string& name );
  explicit Texture( const std::string& p, int w, int h, int n );

  // with index for bindless texture buffer
  explicit Texture( const std::string& p, const std::string& name, uint32_t textureIndex );
  explicit Texture( const std::string& p, int w, int h, int n, uint32_t textureIndex );

  auto getPath() const -> std::string;
  auto getName() const -> std::string;
  auto getWidth() const -> int;
  auto getHeight() const -> int;
  auto getLoaded() const -> bool;

  auto setPath( const std::string& path ) -> void;
  auto setWidth( int width ) -> void;
  auto setHeight( int height ) -> void;
  auto setLoaded( bool value ) -> void;

  auto getImage() -> VkImage&;
  auto getView() -> VkImageView&;
  auto getAllocation() -> VmaAllocation&;

  auto getIndex() const -> uint32_t;
  auto setIndex( uint32_t index ) -> void;
  auto setSamplerIndex( uint32_t index ) -> void;

private:
  graphics::VulkanImage m_image;

  uint32_t m_textureIndex;
  uint32_t m_samplerIndex;

  std::string m_path;
  std::string m_name;
  int m_width{ 0 };
  int m_height{ 0 };
  int m_numComponents{ 0 };
  bool m_loaded{ false };
};
} // namespace resources