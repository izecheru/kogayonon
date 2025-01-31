#pragma once
#include <assimp\Importer.hpp>
// Use the #aiProcess_FlipUVs flag to get UV coordinates with the upper-left corner as origin.

class ModelLoader
{
public:
  ModelLoader() = default;
  ~ModelLoader() = default;

  bool importAsset(const char* path_to_asset);
  bool sceneProcess(const aiScene* scene);

private:
  // assimp importer member variable to load objects into the game
  Assimp::Importer importer;
};
