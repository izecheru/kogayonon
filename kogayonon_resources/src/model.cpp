#include "resources/model.hpp"
#include <filesystem>

namespace kogayonon_resources
{
Model::Model( std::vector<Mesh> meshes )
    : m_meshes{ std::move( meshes ) }
    , m_loaded{ true }
{
}

Model::Model( Model&& other ) noexcept
    : m_loaded{ other.m_loaded }
{
  m_meshes = std::move( other.m_meshes );
}

std::vector<Mesh>& Model::getMeshes()
{
  return m_meshes;
}
} // namespace kogayonon_resources