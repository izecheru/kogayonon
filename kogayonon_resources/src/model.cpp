#include "resources/model.h"
#include <filesystem>

namespace kogayonon_resources {
Model::Model( const std::string& path_to_model )
{
    this->m_path = path_to_model;
    init( path_to_model );
}

Model::Model( Model&& other ) noexcept : m_loaded( other.m_loaded ), m_path( other.m_path )
{
    m_meshes = std::move( other.m_meshes );
}

void Model::init( const std::string& path ) const
{
    // auto mgr = RegistryManager::getFromContext<AssetManager>(Context::AssetManagerContext);
    // mgr->initializeModel(path);
}

std::vector<Mesh>& Model::getMeshes()
{
    return m_meshes;
}
} // namespace kogayonon_resources