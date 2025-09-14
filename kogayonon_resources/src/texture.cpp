#include "resources/texture.h"

namespace kogayonon_resources
{
Texture::Texture(const std::string& p, int w, int h, int n) : m_path(p), m_width(w), m_height(h), m_numComponents(n)
{
}

Texture::Texture(unsigned int id, const std::string& p, int w, int h, int n)
    : m_id(id), m_path(p), m_width(w), m_height(h), m_numComponents(n)
{
}

std::string Texture::getPath() const
{
    return m_path;
}

int Texture::getWidth() const
{
    return m_width;
}

int Texture::getHeight() const
{
    return m_height;
}

unsigned int Texture::getTextureId() const
{
    return m_id;
}

void Texture::setTextureId(unsigned int id)
{
    m_id = id;
}

void Texture::setPath(const std::string& path)
{
    m_path = path;
}

void Texture::setHeight(int height)
{
    m_height = height;
}

void Texture::setWidth(int width)
{
    m_width = width;
}
} // namespace kogayonon_resources