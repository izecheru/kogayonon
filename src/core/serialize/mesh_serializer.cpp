#include "core/serialize/mesh_serializer.h"
#include "core/asset_manager/loader/mesh.h"

namespace kogayonon
{
  void MeshSerializer::serializeMeshes(const std::string& bin_path, Model& model)
  {
    assert(openFile(bin_path, FileMode::WRITE) == true);

    std::vector<Mesh>& meshes = model.getMeshes();

    size_t mesh_count = meshes.size();
    serializeVar(mesh_count);

    serialize(meshes);
    closeFile();
  }

  void MeshSerializer::deserializeMeshes(const std::string& bin_path, Model& model)
  {
    openFile(bin_path, FileMode::READ);
    assert(!isEmptyIn());

    std::vector<Mesh>& meshes = model.getMeshes();

    size_t mesh_count = 0;
    Logger::logInfo(bin_path);
    assert(deserializeVar(mesh_count) == true);

    meshes.resize(mesh_count);

    deserialize(meshes);
    closeFile();
  }

  bool MeshSerializer::serialize(Mesh& mesh)
  {
    std::vector<Vertex>& vertices = mesh.getVertices();
    if(size_t vert_size = vertices.size(); vert_size > 0)
    {
      Serializer::serializeVar(vert_size);
      Serializer::serializeRaw(vertices.data(), vert_size * sizeof(Vertex));
    }
    else
    {
      Logger::logError("Attempting to serialize mesh with 0 vertices");
    }
    std::vector<uint32_t>& indices = mesh.getIndices();
    if(size_t ind_size = indices.size(); ind_size > 0)
    {
      Serializer::serializeVar(ind_size);
      Serializer::serializeRaw(indices.data(), ind_size * sizeof(uint32_t));
    }
    else
    {
      Logger::logError("Attempting to serialize mesh with 0 indices");
    }
    return Serializer::outGood();
  }

  bool MeshSerializer::deserialize(Mesh& mesh)
  {
    // Deserialize vertices
    size_t vertex_count = 0;
    Serializer::deserializeVar(vertex_count);
    std::vector<Vertex> vertices(vertex_count);
    Serializer::deserializeRaw(vertices.data(), vertex_count * sizeof(Vertex));

    // Deserialize indices
    size_t index_count = 0;
    Serializer::deserializeVar(index_count);
    std::vector<uint32_t> indices(index_count);
    Serializer::deserializeRaw(indices.data(), index_count * sizeof(uint32_t));
    mesh = Mesh(vertices, indices);
    return Serializer::inGood();
  }

  bool MeshSerializer::serialize(std::vector<Mesh>& mesh)
  {
    for(auto& m : mesh)
    {
      if(!serialize(m))
      {
        Logger::logError("COuld not serialize mesh");
      }
    }
    return true;
  }

  bool MeshSerializer::deserialize(std::vector<Mesh>& mesh)
  {
    for(auto& m : mesh)
    {
      if(!deserialize(m))
      {
        Logger::logError("COuld not serialize mesh");
      }
    }
    return true;
  }
}