#include "core/ecs/components/texture_component.h"
#include <SOIL2/SOIL2.h>
#include <assert.h>
#include <filesystem>
#include <glad/glad.h>
#include "logger/logger.h"
using namespace kogayonon_logger;

namespace kogayonon_core
{
TextureComponent::TextureComponent(const std::string& textureFileName, const std::string& texturePath) : m_texture(0)
{
    assert(std::filesystem::exists(texturePath) && "Texture file does not exist");
    m_name = textureFileName;
    m_texture = SOIL_load_OGL_texture(texturePath.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
}

unsigned int TextureComponent::getTexture() const
{
    return m_texture;
}

TextureComponent::~TextureComponent()
{
    if (m_texture)
    {
        glDeleteTextures(1, &m_texture);
    }
}
} // namespace kogayonon_core