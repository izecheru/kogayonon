#include "resources/mesh.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_resources
{

Mesh::Mesh( const std::string& path,
            const std::vector<Vertex>&& vertices,
            const std::vector<uint32_t>&& indices,
            const std::vector<Texture*>&& textures,
            const std::vector<Submesh>&& submeshes )
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

Mesh::Mesh( const std::string& path,
            const std::vector<Vertex>&& vertices,
            const std::vector<uint32_t>&& indices,
            const std::vector<Texture*>&& textures,
            const std::vector<Submesh>&& submeshes,
            std::optional<kogayonon_resources::Skeleton> skeleton )
    : m_path{ path }
    , m_vertices{ vertices }
    , m_indices{ indices }
    , m_textures{ textures }
    , m_submeshes{ submeshes }
    , m_vao{ 0 }
    , m_vbo{ 0 }
    , m_ebo{ 0 }
{
  if ( skeleton )
  {
    m_skeleton = *skeleton;
    m_hasSkeleton = true;
  }
}

Mesh::Mesh( const std::string& path,
            const std::vector<Vertex>&& vertices,
            const std::vector<uint32_t>&& indices,
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

auto Mesh::getVertices() -> std::vector<Vertex>&
{
  return m_vertices;
}

auto Mesh::getIndices() -> std::vector<uint32_t>&
{
  return m_indices;
}

auto Mesh::getTextures() -> std::vector<Texture*>&
{
  return m_textures;
}

auto Mesh::getVao() -> uint32_t&
{
  return m_vao;
}

auto Mesh::getVbo() -> uint32_t&
{
  return m_vbo;
}

auto Mesh::getEbo() -> uint32_t&
{
  return m_ebo;
}

auto Mesh::getSubmeshes() -> std::vector<Submesh>&
{
  return m_submeshes;
}

} // namespace kogayonon_resources