#pragma once
#include <assimp\Importer.hpp>
#include "core/renderer/model.h"
// Use the #aiProcess_FlipUVs flag to get UV coordinates with the upper-left corner as origin.

class ModelLoader
{
public:
  static bool importAsset(const char* path_to_asset);
  static bool sceneProcess();
  static Model processModel();

private:
  // assimp importer member variable to load objects into the game
  inline static Assimp::Importer importer;
  inline static const aiScene* m_scene = nullptr;
};
