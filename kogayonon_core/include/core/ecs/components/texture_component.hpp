#pragma once
#include <future>
#include <memory>
#include <string>
#include "resources/texture.hpp"

namespace kogayonon_core
{
struct TextureComponent
{
  explicit TextureComponent( std::weak_ptr<kogayonon_resources::Texture> texture )
      : pTexture{ texture }
  {
  }

  ~TextureComponent() = default;

  inline unsigned int getTextureId() const
  {
    return pTexture.lock()->getTextureId();
  }

  std::weak_ptr<kogayonon_resources::Texture> pTexture;
};
} // namespace kogayonon_core