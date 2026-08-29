#pragma once
#include "precompiled/pch.hpp"

namespace resources
{
enum class BoundType
{
  Atlas,
  Plane
};

struct Bounds
{
  BoundType type;
  float left;
  float bottom;
  float right;
  float top;
};

struct Glyph
{
  char character;
  float advance;
  Bounds atlasBounds;
  Bounds planeBounds;
};

struct AtlasMetrics
{
  uint32_t emSize;
  float lineHeight;
  float ascender;
  float descender;
  float underlineY;
  float underlineThickness;
};

struct FontAtlas
{
  uint32_t width;
  uint32_t height;
  AtlasMetrics metrics;
  std::vector<Glyph> glyphs;
};

class Font
{
public:
  Font( const std::string_view name, const std::string_view jsonPath, const std::string_view pngPath );
  ~Font() = default;

  auto getGlyph( char glyph ) -> const Glyph&;

private:
  std::string m_name;
  std::filesystem::path m_json;
  std::filesystem::path m_png;
  FontAtlas m_atlas;
};
} // namespace resources