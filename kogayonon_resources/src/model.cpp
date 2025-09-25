#include "resources/model.hpp"
#include <filesystem>

namespace kogayonon_resources
{
Model::Model( std::vector<Mesh>&& meshes )
    : m_meshes{ std::move( meshes ) }
{
}

std::vector<Mesh>& Model::getMeshes()
{
  return m_meshes;
}
} // namespace kogayonon_resources