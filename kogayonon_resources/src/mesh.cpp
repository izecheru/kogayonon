#include "resources/mesh.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_resources
{

Mesh::Mesh( const std::string& path, const std::vector<Vertex>&& vertices, const std::vector<uint32_t>&& indices,
            const std::vector<Texture*>&& textures, const std::vector<Submesh>&& submeshes )
    : m_path{ path }
    , m_vertices{ vertices }
    , m_indices{ indices }
    , m_textures{ textures }
    , m_submeshes{ submeshes }
    , m_vao{ 0 }
    , m_vbo{ 0 }
    , m_ebo{ 0 }
{
}

Mesh::Mesh( const std::string& path, const std::vector<Vertex>&& vertices, const std::vector<uint32_t>&& indices,
            const std::vector<Submesh>&& submeshes )
    : m_path{ path }
    , m_vertices{ vertices }
    , m_indices{ indices }
    , m_submeshes{ submeshes }
    , m_vao{ 0 }
    , m_vbo{ 0 }
    , m_ebo{ 0 }
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

std::vector<Texture*>& Mesh::getTextures()
{
  return m_textures;
}

uint32_t& Mesh::getVao()
{
  return m_vao;
}

uint32_t& Mesh::getVbo()
{
  return m_vbo;
}

uint32_t& Mesh::getEbo()
{
  return m_ebo;
}

std::vector<Submesh>& Mesh::getSubmeshes()
{
  return m_submeshes;
}

} // namespace kogayonon_resources