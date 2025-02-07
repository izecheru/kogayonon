#include "core/renderer/model.h"
#include "core/model_loader/model_loader.h"

namespace kogayonon
{


  Model::Model(std::string& path_to_model, Shader& shader) {
    this->path = path_to_model;
    init(path_to_model, shader);
  }

  void Model::render(Shader& shader) {
    for (auto& mesh : m_meshes)
    {
      mesh.render(shader);
    }
  }

  void Model::init(std::string& path, Shader& shader) {
    // since we pass a reference for meshes and textures loaded we don't need the model
    // loader to return a Model object
    ModelLoader::buildModel(path, m_meshes, m_textures_loaded, shader);
  }
}
