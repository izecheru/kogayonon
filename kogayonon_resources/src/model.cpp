#include "resources/model.hpp"
#include <filesystem>
#include <glad/glad.h>
#include <glm/ext/matrix_transform.hpp>

namespace kogayonon_resources
{
Model::Model( std::vector<Mesh>&& meshes, const std::string& path )
    : m_meshes{ std::move( meshes ) }
    , m_path{ path }
{
}

std::string Model::getPath()
{
  return m_path;
}

std::vector<Mesh>& Model::getMeshes()
{
  return m_meshes;
}
} // namespace kogayonon_resources