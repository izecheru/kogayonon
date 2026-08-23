#include "resources/font.hpp"
#include "utilities/utils/utils.hpp"
#include <rapidjson/document.h>

resources::Font::Font( const std::string_view name, const std::string_view jsonPath, const std::string_view pngPath )
    : m_name{ name }
    , m_json{ jsonPath }
    , m_png{ pngPath }
{
  std::ifstream fontFile( m_json );
  std::string json( ( std::istreambuf_iterator<char>( fontFile ) ), std::istreambuf_iterator<char>() );

  rapidjson::Document doc;
  doc.Parse( json.c_str() );

  if ( doc.HasParseError() )
  {
    K_ERROR( "Document has parsing errors" );
  }

  auto metrics = AtlasMetrics{
    .emSize = doc["metrics"]["emSize"].GetUint(),
    .lineHeight = doc["metrics"]["lineHeight"].GetFloat(),
    .ascender = doc["metrics"]["ascender"].GetFloat(),
    .descender = doc["metrics"]["descender"].GetFloat(),
    .underlineY = doc["metrics"]["underlineY"].GetFloat(),
    .underlineThickness = doc["metrics"]["underlineThickness"].GetFloat(),
  };

  const size_t glyphCount = doc["glyphs"].Size();
  std::vector<Glyph> glyphs{};
  glyphs.reserve( glyphCount );
  for ( rapidjson::SizeType i = 0; i < glyphCount; i++ )
  {
    auto& glyph = doc["glyphs"][i];

    if ( i == 0 )
    {
      // the first glyph has no bounds
      continue;
    }
    Bounds atlasBounds{
      .type = BoundType::Atlas,
      .left = glyph["atlasBounds"]["left"].GetFloat(),
      .bottom = glyph["atlasBounds"]["bottom"].GetFloat(),
      .right = glyph["atlasBounds"]["right"].GetFloat(),
      .top = glyph["atlasBounds"]["top"].GetFloat(),
    };

    Bounds planeBounds{
      .type = BoundType::Plane,
      .left = glyph["planeBounds"]["left"].GetFloat(),
      .bottom = glyph["planeBounds"]["bottom"].GetFloat(),
      .right = glyph["planeBounds"]["right"].GetFloat(),
      .top = glyph["planeBounds"]["top"].GetFloat(),
    };

    Glyph g{ .character = static_cast<char>( glyph["unicode"].GetUint() ),
             .advance = glyph["advance"].GetFloat(),
             .atlasBounds = atlasBounds,
             .planeBounds = planeBounds };

    glyphs.emplace_back( g );
  }

  m_atlas = FontAtlas{ .metrics = metrics, .glyphs = std::move( glyphs ) };

  K_INFO( "Atlas has {} glyphs", m_atlas.glyphs.size() );
}

auto resources::Font::getGlyph( char glyph ) -> const Glyph&
{
  for ( auto& g : m_atlas.glyphs )
  {
    if ( g.character == glyph )
    {
      return g;
    }
  }
  throw std::runtime_error( "Glyph could not be found" );
}
