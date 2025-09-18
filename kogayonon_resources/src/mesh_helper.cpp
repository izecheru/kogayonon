#include "resources/mesh_helper.hpp"
#include "resources/vertex.hpp"

namespace kogayonon_resources
{
MeshData::MeshData( std::vector<Vertex>& vertices, std::vector<unsigned int>& indices,
                    std::vector<unsigned int>& textures )
    : m_vertices( std::move( vertices ) )
    , m_indices( std::move( indices ) )
    , m_textures( std::move( textures ) )
{
}

MeshData::MeshData( std::vector<Vertex>& vertices, std::vector<unsigned int>& indices )
    : m_vertices( std::move( vertices ) )
    , m_indices( std::move( indices ) )
{
}
} // namespace kogayonon_resources