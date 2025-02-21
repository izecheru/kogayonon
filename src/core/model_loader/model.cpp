#include "core/model_loader/model.h"
#include "core/model_loader/model_loader.h"

#include <chrono>

namespace kogayonon
{
  Model::Model(const std::string& path_to_model)
  {
    this->path = path_to_model;
    init(path_to_model);
  }

  void Model::draw(Shader& shader)
  {
    if (!m_loaded)
    {
      Logger::logError("Model not yet loaded, skipping draw.");
      return;
    }

    if (m_meshes.empty())
    {
      Logger::logError("No meshes found! Something went wrong with loading.");
      return;
    }

    for (auto& mesh : m_meshes)
    {
      mesh.draw(shader);
    }
  }

  void Model::init(const std::string path)
  {
    ModelLoader::buildModel(path, m_meshes, m_textures_loaded);
    m_loaded = true;
  }

  std::vector<Mesh>& Model::getMeshes()
  {
    return m_meshes;
  }

  void Model::operator=(const Model& other)
  {
    // maybe not needed this atomic thing
    this->m_loaded = other.m_loaded;
    this->m_meshes = other.m_meshes;
    this->m_textures_loaded = other.m_textures_loaded;
  }
}
