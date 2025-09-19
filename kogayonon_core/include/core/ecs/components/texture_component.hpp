#pragma once
#include <future>
#include <memory>
#include <string>
#include "resources/texture.hpp"

namespace kogayonon_core
{
struct TextureComponent
{
  explicit TextureComponent( std::weak_ptr<kogayonon_resources::Texture> texture );
  ~TextureComponent();

  unsigned int getTextureId();
  std::weak_ptr<kogayonon_resources::Texture> pTexture;
};
} // namespace kogayonon_core