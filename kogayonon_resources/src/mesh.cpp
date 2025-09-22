#include "resources/mesh.hpp"
#include "resources/mesh_helper.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_resources
{
kogayonon_resources::Mesh::Mesh( std::vector<Vertex> vertices, std::vector<unsigned int> indices,
                                 std::vector<unsigned int> textures )
    : m_data( std::make_shared<MeshData>( std::move( vertices ), std::move( indices ), std::move( textures ) ) )
{
}

kogayonon_resources::Mesh::Mesh( std::vector<Vertex> vertices, std::vector<unsigned int> indices )
    : m_data( std::make_shared<MeshData>( std::move( vertices ), std::move( indices ) ) )
{
}

std::vector<Vertex>& Mesh::getVertices()
{
  return m_data->m_vertices;
}

std::vector<uint32_t>& Mesh::getIndices()
{
  return m_data->m_indices;
}

std::vector<unsigned int>& Mesh::getTextures()
{
  return m_data->m_textures;
}

Mesh::Mesh( const Mesh& other )
{
  m_data = std::make_shared<MeshData>( *other.m_data );
  m_gpu = std::make_shared<MeshGPU>( *other.m_gpu );
  m_init = other.m_init;
}

Mesh& Mesh::operator=( Mesh&& other ) noexcept
{
  if ( this != &other )
  {
    this->m_data = std::move( other.m_data );
    this->m_gpu = std::move( other.m_gpu );
    this->m_init = other.m_init;
  }
  return *this;
}

Mesh& Mesh::operator=( const Mesh& other )
{
  if ( this != &other )
  {
    m_data = std::make_shared<MeshData>( *other.m_data );
    m_gpu = std::make_shared<MeshGPU>( *other.m_gpu );
    m_init = other.m_init;
  }
  return *this;
}
} // namespace kogayonon_resources