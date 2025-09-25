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

Mesh::Mesh( const Mesh& other )
    : m_textures( other.m_textures )
    , m_vertices( other.m_vertices )
    , m_indices( other.m_indices )
    , m_init( other.m_init )
{
}

Mesh& Mesh::operator=( const Mesh& other )
{
  if ( this == &other )
    return *this;
  m_textures = other.m_textures;
  m_vertices = other.m_vertices;
  m_indices = other.m_indices;
  m_init = other.m_init;
  return *this;
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

unsigned int& Mesh::getVao()
{
  return vao;
}

unsigned int& Mesh::getVbo()
{
  return vbo;
}

unsigned int& Mesh::getEbo()
{
  return ebo;
}
} // namespace kogayonon_resources