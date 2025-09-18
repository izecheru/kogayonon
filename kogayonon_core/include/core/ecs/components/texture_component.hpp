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

  unsigned int getTexture();
  std::weak_ptr<kogayonon_resources::Texture> m_texture;
};
} // namespace kogayonon_core