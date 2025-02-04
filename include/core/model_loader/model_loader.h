#pragma once
#include <assimp\Importer.hpp>
#include "core/renderer/model.h"
// Use the #aiProcess_FlipUVs flag to get UV coordinates with the upper-left corner as origin.

class ModelLoader
{
public:
  static const aiScene* getScene(const char* path_to_asset);

  static Mesh processMesh(const aiMesh* mesh);
  static void processNode(std::vector<Mesh>& meshes, aiNode* node, const aiScene* scene);

private:
  // assimp importer member variable to load objects into the game
  inline static Assimp::Importer importer;
};
