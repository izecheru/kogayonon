#include "resources/texture.hpp"

namespace resources
{

Texture::Texture( const std::string& p, const std::string& name )
    : m_path{ p }
    , m_width{ 0 }
    , m_height{ 0 }
    , m_name{ name }
    , m_numComponents{ 0 }
    , m_loaded{ false }
{
}

Texture::Texture( const std::string& p, int w, int h, int n )
    : m_path{ p }
    , m_width{ w }
    , m_height{ h }
    , m_numComponents{ n }
{
}

auto Texture::getPath() const -> std::string
{
  return m_path;
}

auto Texture::getName() const -> std::string
{
  return m_name;
}

auto Texture::getWidth() const -> int
{
  return m_width;
}

auto Texture::getHeight() const -> int
{
  return m_height;
}

void Texture::setPath( const std::string& path )
{
  m_path = path;
}

void Texture::setHeight( int height )
{
  m_height = height;
}

void Texture::setWidth( int width )
{
  m_width = width;
}

bool Texture::getLoaded() const
{
  return m_loaded;
}

void Texture::setLoaded( bool value )
{
  m_loaded = value;
}

auto Texture::getImage() -> VkImage&
{
  return m_image;
}

auto Texture::getView() -> VkImageView&
{
  return m_imageView;
}

auto Texture::getMemory() -> VkDeviceMemory&
{
  return m_imageMemory;
}

} // namespace resources