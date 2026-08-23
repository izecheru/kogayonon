#pragma once
#include <fstream>
#include "msdf-atlas-gen/msdf-atlas-gen.h"

namespace core
{

class FontLoader
{
public:
  FontLoader();
  ~FontLoader();

  void generateAtlas( const std::string_view path );

private:
  msdf_atlas::Charset m_defaultCharset;
  msdfgen::FreetypeHandle* m_pFreetypeHandle;
};
} // namespace core