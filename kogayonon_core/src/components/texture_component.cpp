#include "core/ecs/components/texture_component.hpp"

namespace kogayonon_core
{
TextureComponent::TextureComponent( std::weak_ptr<kogayonon_resources::Texture> texture )
    : pTexture( texture )
{
}

unsigned int TextureComponent::getTextureId()
{
  return pTexture.lock()->getTextureId();
}

TextureComponent::~TextureComponent()
{
}
} // namespace kogayonon_core