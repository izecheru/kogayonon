#pragma once
#include "precompiled/pch.hpp"
#include "resources/texture.hpp"

namespace core
{
struct TextureComponent
{
  explicit TextureComponent( std::weak_ptr<resources::Texture> texture )
      : pTexture{ texture }
  {
  }

  ~TextureComponent() = default;

  std::weak_ptr<resources::Texture> pTexture;
};
} // namespace core