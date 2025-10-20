#include "resources/texture.hpp"

namespace kogayonon_resources
{
Texture::Texture( const std::string& p, int w, int h, int n )
    : m_path{ p }
    , m_width{ w }
    , m_height{ h }
    , m_numComponents{ n }
{
}

Texture::Texture( uint32_t id, const std::string& p, const std::string& name, int w, int h, int n )
    : m_id{ id }
    , m_path{ p }
    , m_width{ w }
    , m_height{ h }
    , m_numComponents{ n }
    , m_name{ name }
{
}

std::string Texture::getPath() const
{
  return m_path;
}

std::string Texture::getName() const
{
  return m_name;
}

int Texture::getWidth() const
{
  return m_width;
}

int Texture::getHeight() const
{
  return m_height;
}

uint32_t Texture::getTextureId() const
{
  return m_id;
}

void Texture::setTextureId( uint32_t id )
{
  m_id = id;
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
} // namespace kogayonon_resources