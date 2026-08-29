#pragma once
#include "precompiled/pch.hpp"
#include "resources/font.hpp"

namespace core
{
struct TextComponent
{
  std::string text;
  resources::Font* pFont;
};
} // namespace core
