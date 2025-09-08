#pragma once
#include <string>

namespace kogayonon_core
{
struct TextureComponent
{
    explicit TextureComponent(const std::string& textureFileName, const std::string& texturePath);
    ~TextureComponent();

    unsigned int getTexture() const;

    unsigned int m_texture;
    std::string m_name;
    int m_w, m_h, m_channels;
};
} // namespace kogayonon_core