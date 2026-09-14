#include "resources/mesh.hpp"
#include <vma/vk_mem_alloc.h>
#include "resources/vertex.hpp"

auto resources::Mesh::getVertices() -> std::vector<Vertex>&
{
  return m_vertices;
}

auto resources::Mesh::getIndices() -> std::vector<uint32_t>&
{
  return m_indices;
}

auto resources::Mesh::getVertexBufferObject() -> graphics::VulkanBuffer&
{
  return m_verticesBuff;
}

auto resources::Mesh::getVerticesAllocation() -> VmaAllocation&
{
  return m_verticesBuff.vmaAllocation;
}

auto resources::Mesh::getIndicesBufferObject() -> graphics::VulkanBuffer&
{
  return m_indicesBuff;
}

auto resources::Mesh::getIndicesAllocation() -> VmaAllocation&
{
  return m_indicesBuff.vmaAllocation;
}

auto resources::Mesh::getSubmeshes() -> std::vector<Submesh>&
{
  return m_submeshes;
}
