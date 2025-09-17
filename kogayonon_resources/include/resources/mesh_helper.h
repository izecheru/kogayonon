#pragma once
#include <string>
#include <vector>

namespace kogayonon_resources
{
struct Vertex;

struct MeshData
{
  std::vector<Vertex> m_vertices{};
  std::vector<unsigned int> m_indices{};
  std::vector<unsigned int> m_textures{};

  explicit MeshData( std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                     std::vector<unsigned int>& textures );

  explicit MeshData( std::vector<Vertex>& vertices, std::vector<unsigned int>& indices );

  ~MeshData()
  {
  }
};
} // namespace kogayonon_resources