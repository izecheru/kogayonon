#include "resources/mesh.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_resources
{
Mesh::Mesh( std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices, std::vector<uint32_t>&& textures )
    : m_texturesIDs{ std::move( textures ) }
    , m_vertices{ std::move( vertices ) }
    , m_indices{ std::move( indices ) }
    , vao{ 0 }
    , vbo{ 0 }
    , ebo{ 0 }
{
}

Mesh::Mesh( std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices )
    : m_vertices{ std::move( vertices ) }
    , m_indices{ std::move( indices ) }
    , vao{ 0 }
    , vbo{ 0 }
    , ebo{ 0 }

{
}

Mesh::Mesh( const Mesh& other )
    : m_texturesIDs( other.m_texturesIDs )
    , m_vertices( other.m_vertices )
    , m_indices( other.m_indices )
    , vao{ other.vao }
    , vbo{ other.vbo }
    , ebo{ other.ebo }
{
}

Mesh& Mesh::operator=( const Mesh& other )
{
  if ( this == &other )
    return *this;
  m_texturesIDs = other.m_texturesIDs;
  m_vertices = other.m_vertices;
  m_indices = other.m_indices;
  vao = other.vao;
  vbo = other.vbo;
  ebo = other.ebo;
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

std::vector<uint32_t>& Mesh::getTextures()
{
  return m_texturesIDs;
}

uint32_t& Mesh::getVao()
{
  return vao;
}

uint32_t& Mesh::getVbo()
{
  return vbo;
}

uint32_t& Mesh::getEbo()
{
  return ebo;
}
} // namespace kogayonon_resources