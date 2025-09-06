#include "resources/mesh_helper.h"
#include "resources/vertex.h"

namespace kogayonon_resources {
MeshData::MeshData( std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices,
                    std::vector<std::string>&& textures )
    : m_vertices( std::move( vertices ) ), m_indices( std::move( indices ) ), m_textures( std::move( textures ) )
{}

MeshData::MeshData( std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices )
    : m_vertices( std::move( vertices ) ), m_indices( std::move( indices ) )
{}
} // namespace kogayonon_resources