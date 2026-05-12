#include "resources/mesh.hpp"
#include <vma/vk_mem_alloc.h>
#include "resources/vertex.hpp"

resources::Mesh::Mesh( const std::string& path,
                       const std::vector<Vertex>&& vertices,
                       const std::vector<uint32_t>&& indices,
                       const std::vector<Texture*>&& textures )
    : m_path{ path }
    , m_vertices{ vertices }
    , m_indices{ indices }
    , m_textures{ textures }
{
}

resources::Mesh::Mesh( const std::string& path,
                       const std::vector<Vertex>&& vertices,
                       const std::vector<uint32_t>&& indices )
    : m_path{ path }
    , m_vertices{ vertices }
    , m_indices{ indices }
{
}

auto resources::Mesh::getVertices() -> std::vector<Vertex>&
{
  return m_vertices;
}

auto resources::Mesh::getIndices() -> std::vector<uint32_t>&
{
  return m_indices;
}

auto resources::Mesh::getTextures() -> std::vector<Texture*>&
{
  return m_textures;
}

auto resources::Mesh::getVertexBufferObject() -> graphics::GpuBuffer&
{
  return m_verticesBuff;
}

auto resources::Mesh::getVerticesAllocation() -> VmaAllocation&
{
  return m_verticesBuff.allocation;
}

auto resources::Mesh::getIndicesBufferObject() -> graphics::GpuBuffer&
{
  return m_indicesBuff;
}

auto resources::Mesh::getIndicesAllocation() -> VmaAllocation&
{
  return m_indicesBuff.allocation;
}

auto resources::Mesh::getSubmeshes() -> std::vector<Submesh>&
{
  return m_submeshes;
}
