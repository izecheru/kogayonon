#include "resources/texture.hpp"

resources::Texture::Texture( const std::string& p, const std::string& name )
    : m_path{ p }
    , m_width{ 0 }
    , m_height{ 0 }
    , m_name{ name }
    , m_numComponents{ 0 }
    , m_loaded{ false }
{
}

resources::Texture::Texture( const std::string& p, int w, int h, int n )
    : m_path{ p }
    , m_width{ w }
    , m_height{ h }
    , m_numComponents{ n }
{
}

resources::Texture::Texture( const std::string& p, const std::string& name, uint32_t textureIndex )
    : m_path{ p }
    , m_width{ 0 }
    , m_height{ 0 }
    , m_name{ name }
    , m_numComponents{ 0 }
    , m_loaded{ false }
    , m_textureIndex{ textureIndex }
{
}

resources::Texture::Texture( const std::string& p, int w, int h, int n, uint32_t textureIndex )
    : m_path{ p }
    , m_width{ w }
    , m_height{ h }
    , m_numComponents{ n }
    , m_textureIndex{ textureIndex }
{
}

auto resources::Texture::getPath() const -> std::string
{
  return m_path;
}

auto resources::Texture::getName() const -> std::string
{
  return m_name;
}

auto resources::Texture::getWidth() const -> int
{
  return m_width;
}

auto resources::Texture::getHeight() const -> int
{
  return m_height;
}

void resources::Texture::setPath( const std::string& path )
{
  m_path = path;
}

void resources::Texture::setHeight( int height )
{
  m_height = height;
}

void resources::Texture::setWidth( int width )
{
  m_width = width;
}

bool resources::Texture::getLoaded() const
{
  return m_loaded;
}

void resources::Texture::setLoaded( bool value )
{
  m_loaded = value;
}

auto resources::Texture::getImage() -> VkImage&
{
  return m_image.vkImage;
}

auto resources::Texture::getView() -> VkImageView&
{
  return m_image.vkImageView;
}

auto resources::Texture::getAllocation() -> VmaAllocation&
{
  return m_image.vmaAllocation;
}

auto resources::Texture::getIndex() const -> uint32_t
{
  return m_textureIndex;
}

void resources::Texture::setIndex( uint32_t index )
{
  m_textureIndex = index;
}

auto resources::Texture::setSamplerIndex( uint32_t index ) -> void
{
  m_samplerIndex = index;
}