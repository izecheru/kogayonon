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

  void Model::draw(Shader& shader)
  {
    if (!m_loaded)
    {
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
    ModelLoader::getInstance().parseGltf(path, m_meshes);
  }

  void Model::serializeMeshes(const std::string& path)
  {
    MeshSerializer& serializer = MeshSerializer::getInstance();
    serializer.openFile(path, FileMode::WRITE);

    size_t mesh_count = m_meshes.size();
    serializer.serializeVar(mesh_count);

    serializer.serialize(m_meshes);
    serializer.closeFile();
  }

  void Model::deserializeMeshes(const std::string& path)
  {
    MeshSerializer& serializer = MeshSerializer::getInstance();
    serializer.openFile(path, FileMode::READ);

    size_t mesh_count = 0;
    serializer.deserializeVar(mesh_count);

    m_meshes.resize(mesh_count);

    serializer.deserialize(m_meshes);
    serializer.closeFile();
  }

  std::vector<Mesh>& Model::getMeshes()
  {
    return m_meshes;
  }

  void Model::operator=(const Model& other)
  {
    this->m_loaded = other.m_loaded;
    this->m_meshes = other.m_meshes;
    this->m_textures_loaded = other.m_textures_loaded;
  }
}
