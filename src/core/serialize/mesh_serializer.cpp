#include "core/serialize/mesh_serializer.h"
#include "core/model_loader/mesh.h"

namespace kogayonon
{

  bool MeshSerializer::serialize(std::ofstream& out, Mesh& mesh)
  {
    std::vector<Vertex>& vertices = mesh.getVertices();
    size_t vert_size = vertices.size();
    if (vert_size > 0)
    {
      out.write(reinterpret_cast<const char*>(&vert_size), sizeof(vert_size));
      out.write(reinterpret_cast<const char*>(vertices.data()), vert_size * sizeof(Vertex));
    }
    else
    {
      Logger::logError("Attempting to serialize mesh with 0 vertices");
    }

    std::vector<uint32_t>& indices = mesh.getIndices();
    size_t ind_size = indices.size();
    if (ind_size > 0)
    {
      out.write(reinterpret_cast<const char*>(&ind_size), sizeof(ind_size));
      out.write(reinterpret_cast<const char*>(indices.data()), ind_size * sizeof(uint32_t));
    }
    else
    {
      Logger::logError("Attempting to serialize mesh with 0 indices");
    }

    return out.good();
  }

  bool MeshSerializer::deserialize(std::ifstream& in, Mesh& mesh)
  {
    // Deserialize vertices
    size_t vertex_count = 0;
    in.read(reinterpret_cast<char*>(&vertex_count), sizeof(vertex_count));
    std::vector<Vertex> vertices(vertex_count);
    in.read(reinterpret_cast<char*>(vertices.data()), vertex_count * sizeof(Vertex));

    // Deserialize indices
    size_t index_count = 0;
    in.read(reinterpret_cast<char*>(&index_count), sizeof(index_count));
    std::vector<uint32_t> indices(index_count);
    in.read(reinterpret_cast<char*>(indices.data()), index_count * sizeof(uint32_t));

    // Deserialize textures
    //size_t texture_count = 0;
    //in.read(reinterpret_cast<char*>(&texture_count), sizeof(texture_count));
    //std::vector<Texture> textures(texture_count);
    //in.read(reinterpret_cast<char*>(textures.data()), texture_count * sizeof(Texture));

    // Rebuild Mesh

    std::vector<Texture> textures(0);
    mesh = Mesh(vertices, indices, textures);
    //mesh.setupMesh(); // Ensure VAOs/VBOs are re-initialized

    return in.good();
  }
}
