#include "core/asset_manager/loader/model.h"
#include "core/asset_manager/loader/model_loader.h"
#include "core/asset_manager/asset_manager.h"
#include "core/serialize/mesh_serializer.h"

#include <filesystem>

namespace kogayonon
{
  Model::Model(const std::string& path_to_model)
  {
    this->m_path = path_to_model;
    init(path_to_model);
  }

  Model::Model(Model&& other)noexcept
  {
    m_loaded = other.m_loaded;
    m_meshes = other.m_meshes;
    m_path = other.m_path;
  }

  void Model::init(const std::string& path)const
  {
    AssetManager::getInstance().initializeModel(path);
  }

  std::vector<Mesh>& Model::getMeshes()
  {
    return m_meshes;
  }

  void Model::operator=(const Model& other)
  {
    this->m_loaded = other.m_loaded;
    this->m_meshes = other.m_meshes;
  }
}