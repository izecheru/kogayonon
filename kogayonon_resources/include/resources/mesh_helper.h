#pragma once
#include <string>
#include <vector>

namespace kogayonon_resources {
struct Vertex;

struct MeshData
{
    std::vector<Vertex> m_vertices{};
    std::vector<uint32_t> m_indices{};
    std::vector<std::string> m_textures{};

    explicit MeshData(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices, std::vector<std::string>&& textures);
    explicit MeshData(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);

    ~MeshData() {}
};

} // namespace kogayonon_resources