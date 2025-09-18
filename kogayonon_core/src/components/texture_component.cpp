#include "core/ecs/components/texture_component.hpp"

namespace kogayonon_core
{
TextureComponent::TextureComponent( std::weak_ptr<kogayonon_resources::Texture> texture )
    : m_texture( texture )
{
}

unsigned int TextureComponent::getTexture()
{
  return m_texture.lock()->getTextureId();
}

TextureComponent::~TextureComponent()
{
}
} // namespace kogayonon_core