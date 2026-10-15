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

auto resources::Mesh::getSubmeshes() -> std::vector<Submesh>&
{
    return m_submeshes;
}
