#include "serialiser/mesh_serialiser.h"

#include "asset_manager/loader/mesh.h"
#include "context_manager/context_manager.h"

namespace kogayonon
{
void MeshSerializer::serializeMeshes(const std::string& bin_path, Model& model, std::ofstream& out)
{
  openFile(bin_path, out, FileMode::WRITE);
  std::vector<Mesh>& meshes = model.getMeshes();

  size_t mesh_count = meshes.size();
  serializeVar(mesh_count, out);

  serialize(meshes, out);
  closeFile(out);
}

void MeshSerializer::deserializeMeshes(const std::string& bin_path, Model& model, std::ifstream& in)
{
  openFile(bin_path, in, FileMode::READ);

  std::vector<Mesh>& meshes = model.getMeshes();

  size_t mesh_count = 0;
  assert(deserializeVar(mesh_count, in) == true);

  meshes.resize(mesh_count);

  deserialize(meshes, in);
  closeFile(in);
}

bool MeshSerializer::serialize(Mesh& mesh, std::ofstream& out)
{
  std::vector<Vertex>& vertices = mesh.getVertices();
  if (size_t vert_size = vertices.size(); vert_size > 0)
  {
    Serializer::serializeVar(vert_size, out);
    Serializer::serializeRaw(vertices.data(), vert_size * sizeof(Vertex), out);
  }
  else
  {
    ContextManager::klogger()->log(LogType::ERROR, "Attempting to serialize mesh with 0 vertices");
    return false;
  }
  std::vector<uint32_t>& indices = mesh.getIndices();
  if (size_t ind_size = indices.size(); ind_size > 0)
  {
    Serializer::serializeVar(ind_size, out);
    Serializer::serializeRaw(indices.data(), ind_size * sizeof(uint32_t), out);
  }
  else
  {
    ContextManager::klogger()->log(LogType::ERROR, "Attempting to serialize mesh with 0 indices");
    return false;
  }
  return true;
}

bool MeshSerializer::deserialize(Mesh& mesh, std::ifstream& in)
{
  // Deserialize vertices
  size_t vertex_count = 0;
  Serializer::deserializeVar(vertex_count, in);
  std::vector<Vertex> vertices(vertex_count);
  Serializer::deserializeRaw(vertices.data(), vertex_count * sizeof(Vertex), in);

  // Deserialize indices
  size_t index_count = 0;
  Serializer::deserializeVar(index_count, in);
  std::vector<uint32_t> indices(index_count);
  Serializer::deserializeRaw(indices.data(), index_count * sizeof(uint32_t), in);
  mesh = Mesh(std::move(vertices), std::move(indices));
  return Serializer::isGood(in);
}

bool MeshSerializer::serialize(std::vector<Mesh>& mesh, std::ofstream& out)
{
  for (auto& m : mesh)
  {
    if (!serialize(m, out))
    {
      ContextManager::klogger()->log(LogType::ERROR, "Could not serialize mesh");
    }
  }
  return true;
}

bool MeshSerializer::deserialize(std::vector<Mesh>& mesh, std::ifstream& in)
{
  for (auto& m : mesh)
  {
    if (!deserialize(m, in))
    {
      ContextManager::klogger()->log(LogType::ERROR, "Could not deserialize mesh");
    }
  }
  return true;
}
} // namespace kogayonon