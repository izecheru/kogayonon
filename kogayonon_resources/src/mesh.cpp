#include "resources/mesh.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_resources
{
Mesh::Mesh( std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices, std::vector<unsigned int>&& textures )
    : m_textures{ std::move( textures ) }
    , m_vertices{ std::move( vertices ) }
    , m_indices{ std::move( indices ) }
{
}

Mesh::Mesh( std::vector<Vertex>&& vertices, std::vector<unsigned int>&& indices )
    : m_vertices{ std::move( vertices ) }
    , m_indices{ std::move( indices ) }
{
}

std::vector<Vertex>& Mesh::getVertices()
{
  return m_vertices;
}

std::vector<uint32_t>& Mesh::getIndices()
{
  return m_indices;
}

std::vector<unsigned int>& Mesh::getTextures()
{
  return m_textures;
}
} // namespace kogayonon_resources