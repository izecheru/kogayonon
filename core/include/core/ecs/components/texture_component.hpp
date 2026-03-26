#pragma once
#include <future>
#include <memory>
#include <string>
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

  inline unsigned int getTextureId() const
  {
    return pTexture.lock()->getTextureId();
  }

  std::weak_ptr<resources::Texture> pTexture;
};
} // namespace core