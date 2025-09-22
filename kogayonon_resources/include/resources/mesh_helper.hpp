#pragma once
#include <string>
#include <vector>

namespace kogayonon_resources
{
struct Vertex;

class MeshData
{
public:
  std::vector<Vertex> m_vertices{};
  std::vector<unsigned int> m_indices{};
  std::vector<unsigned int> m_textures{};

  explicit MeshData( std::vector<Vertex> vertices, std::vector<unsigned int> indices,
                     std::vector<unsigned int> textures )
      : m_vertices( std::move( vertices ) )
      , m_indices( std::move( indices ) )
      , m_textures( std::move( textures ) )
  {
  }

  explicit MeshData( std::vector<Vertex> vertices, std::vector<unsigned int> indices )
      : m_vertices( std::move( vertices ) )
      , m_indices( std::move( indices ) )
  {
  }
};
} // namespace kogayonon_resources