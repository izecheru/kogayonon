#include "core/model_loader/model.h"
#include "core/model_loader/model_loader.h"
#include "core/serialize/mesh_serializer.h"

#include <filesystem>

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
    ModelLoader::getInstance().buildModel(path, m_meshes);
    m_loaded = true;
    // TODO make a name or something for each model so that we dont serialize those already serialized
    if (std::filesystem::exists("resources/models/serialized/out.bin")) return;
    serializeMeshes("resources/models/serialized/out.bin");
  }

  void Model::serializeMeshes(const std::string& path)
  {
    if (!std::filesystem::exists(path))
    {
      std::filesystem::create_directories(std::filesystem::path(path).parent_path());
    }
    std::ofstream out(path, std::ios::binary);
    assert(out.is_open());
    size_t mesh_count = m_meshes.size();
    out.write(reinterpret_cast<const char*>(&mesh_count), sizeof(mesh_count));

    for (auto& mesh : m_meshes)
    {
      if (!MeshSerializer::getInstance().serialize(out, mesh))
      {
        Logger::logError("could not serialize mesh");
      }
    }
    out.close();
  }

  void Model::deserializeMeshes(const std::string& path)
  {
    std::ifstream in(path, std::ios::binary);
    size_t mesh_count = 0;
    in.read(reinterpret_cast<char*>(&mesh_count), sizeof(mesh_count));
    m_meshes.resize(mesh_count);

    for (int i = 0; i < mesh_count; i++)
    {
      MeshSerializer::getInstance().deserialize(in, m_meshes[i]);
    }
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
