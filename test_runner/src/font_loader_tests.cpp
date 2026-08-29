#include "core/asset_manager/font_loader.hpp"
#include <Windows.h>
#include <gtest/gtest.h>

namespace core
{
TEST( CoreAssetManager, GenerateAtlasTest )
{
  auto fontLoader = std::make_shared<core::FontLoader>();
  auto fontPath = std::filesystem::path{
    "F:\\github\\kogayonon\\build\\x64-debug\\bin\\engine_resources\\fonts\\Inter_18pt-Black.ttf" };
  EXPECT_EQ( std::filesystem::exists( fontPath ), true );
  fontLoader->generateAtlas( fontPath.string() );
  auto pngName = fontPath.stem().string() + ".png";
  auto jsonName = fontPath.stem().string() + ".json";
  auto fontDir = fontPath.parent_path();
  auto existsPng = std::filesystem::exists( fontDir / pngName );
  auto existsJson = std::filesystem::exists( fontDir / jsonName );
  EXPECT_EQ( existsPng, true );
  EXPECT_EQ( existsJson, true );
}
} // namespace core