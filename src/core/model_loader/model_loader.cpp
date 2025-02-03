#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "core/model_loader/model_loader.h"
#include "core/logger.h"
#include "core/renderer/model.h"

bool ModelLoader::importAsset(const char* path_to_asset) {
  m_scene = importer.ReadFile(path_to_asset,
    aiProcess_CalcTangentSpace |
    aiProcess_Triangulate |
    aiProcess_JoinIdenticalVertices |
    aiProcess_SortByPType);
  if (m_scene == nullptr || m_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
  {
    Logger::logError("Could not load object from [", path_to_asset, "]");
    return false;
  }
  Logger::logInfo("Loaded model ", path_to_asset);
  sceneProcess();
  return true;
}

bool ModelLoader::sceneProcess() {
  if (!m_scene->HasMeshes())
  {
    Logger::logError("No meshes present in the scene!!!");
  }
  else
  {
    Logger::logInfo("meshes:", m_scene->mNumMeshes);
    Logger::logInfo("animations:", m_scene->mNumAnimations);
    Logger::logInfo("materials:", m_scene->mNumMaterials);
    Logger::logInfo("textures:", m_scene->mNumTextures);
    for (int i = 0; i < m_scene->mNumMeshes; i++)
    {
      for (int j = 0; j < m_scene->mMeshes[i]->mNumBones; j++)
      {
        Logger::logInfo("name of bone for mesh ", i, ", bone number :", j, " ", m_scene->mMeshes[i]->mBones[j]->mName.C_Str());
      }
    }
  }
  return false;
}

Model ModelLoader::processModel() {
  std::vector<Mesh> meshes(m_scene->mNumMeshes);
  Mesh temp_mesh;
  for (int i = 0; i < m_scene->mNumMeshes; i++)
  {
    meshes.push_back();
  }
  return Model("path", m_scene->mMeshes);
}
