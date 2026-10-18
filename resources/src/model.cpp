#include "resources/model.hpp"
#include <vma/vk_mem_alloc.h>
#include "resources/vertex.hpp"

auto resources::Model::getVertices() -> std::vector<Vertex>&
{
    return m_vertices;
}

auto resources::Model::getIndices() -> std::vector<uint32_t>&
{
    return m_indices;
}

auto resources::Model::getMeshes() -> std::vector<Mesh>&
{
    return m_meshes;
}
