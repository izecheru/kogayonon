#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "core/model_loader/model_loader.h"
#include "core/logger.h"

bool ModelLoader::importAsset(const char* path_to_asset) {
  const aiScene* scene = importer.ReadFile(path_to_asset,
    aiProcess_CalcTangentSpace |
    aiProcess_Triangulate |
    aiProcess_JoinIdenticalVertices |
    aiProcess_SortByPType);
  if (scene == nullptr)
  {
    Logger::logError("Could not load object from [", path_to_asset, "]");
    return false;
  }
  sceneProcess(scene);
  return true;
}

bool ModelLoader::sceneProcess(const aiScene* scene) {
  if (scene != nullptr)
  {
    if (!scene->HasMeshes())
    {
      Logger::logError("No meshes present in the scene!!!");
    }
    else
    {
      Logger::logInfo("meshes:", scene->mNumMeshes);
      Logger::logInfo("animations:", scene->mNumAnimations);
      Logger::logInfo("materials:", scene->mNumMaterials);
      Logger::logInfo("textures:", scene->mNumTextures);
      for (int i = 0; i < scene->mNumMeshes; i++)
      {
        Logger::logInfo("bones for mesh ", i, ":", scene->mMeshes[i]->mNumBones);
      }
    }
  }
  return false;
}
