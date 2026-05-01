#pragma once
#include <string>
#include <vector>
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
  // explicit Texture(
  //   uint32_t id, const std::string& p, const std::string& name, int w, int h, int n, uint32_t textureIndex );

  std::string getPath() const;
  std::string getName() const;
  int getWidth() const;
  int getHeight() const;
  bool getLoaded() const;

  void setPath( const std::string& path );
  void setWidth( int width );
  void setHeight( int height );
  void setLoaded( bool value );

  auto getImage() -> VkImage&;
  auto getView() -> VkImageView&;
  auto getAllocation() -> VmaAllocation&;

  auto getIndex() const -> uint32_t;
  void setIndex( uint32_t index );

private:
  VkImage m_image;
  VkImageView m_imageView;
  VmaAllocation m_imageAllocation;

  // when we have the same mesh for two submeshes, we check to see if the texture is already loaded
  // and then retrieve the index and assign it to the coresponding material member variable
  uint32_t m_textureIndex;

  std::string m_path;
  std::string m_name;
  int m_width{ 0 };
  int m_height{ 0 };
  int m_numComponents{ 0 };
  bool m_loaded{ false };
};
} // namespace resources