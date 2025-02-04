#include "core/renderer/model.h"
#include "core/model_loader/model_loader.h"

Model::Model(const char* path_to_model) {
  this->path = path_to_model;
  loadModel(path_to_model);
}

void Model::draw(Shader& shader) {
  for (auto& it : m_meshes)
  {
    it.draw();
  }
}

void Model::loadModel(const char* path) {
  const aiScene* scene = ModelLoader::getScene(path);
  if (scene != nullptr)
  {
    ModelLoader::processNode(m_meshes, scene->mRootNode, scene);
  }
}

