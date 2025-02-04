#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "core/model_loader/model_loader.h"
#include "core/logger.h"
#include "core/renderer/model.h"

const aiScene* ModelLoader::getScene(const char* path_to_asset) {
  const aiScene* scene = importer.ReadFile(path_to_asset,
    aiProcess_CalcTangentSpace |
    aiProcess_Triangulate |
    aiProcess_SortByPType);
  if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
  {
    Logger::logError("Could not load object from [", path_to_asset, "]");
    return nullptr;
  }
  return scene;
}

Mesh ModelLoader::processMesh(const aiMesh* mesh) {
  std::vector<Vertex> vertices;
  std::vector<Texture> textures;
  std::vector<unsigned int> indices;

  for (unsigned int i = 0; i < mesh->mNumVertices; i++)
  {
    Vertex vertex;
    vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
    vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

    if (mesh->mTextureCoords[0])
    {
      vertex.texture = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
    }
    else
    {
      vertex.texture = glm::vec2(0.0f, 0.0f);
    }

    vertices.push_back(vertex);
  }

  for (unsigned int i = 0; i < mesh->mNumFaces; i++)
  {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
    {
      indices.push_back(face.mIndices[j]);
    }
  }
  Logger::logInfo("processed ", vertices.size(), " vertices and ", indices.size(), " indices");
  return Mesh(vertices, indices);
}

void ModelLoader::processNode(std::vector<Mesh>& meshes, aiNode* node, const aiScene* scene) {
  for (unsigned int i = 0; i < node->mNumMeshes; i++)
  {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh));
  }

  for (unsigned int i = 0; i < node->mNumChildren; i++)
  {
    processNode(meshes, node->mChildren[i], scene);
  }
}
