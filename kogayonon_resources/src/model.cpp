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

glm::vec2 Model::getAABB()
{
  auto minAABB = glm::vec3( std::numeric_limits<float>::max() );
  auto maxAABB = glm::vec3( std::numeric_limits<float>::lowest() );

  for ( auto& mesh : m_meshes )
  {
    const auto& vertices = mesh.getVertices();
    for ( const auto& vertex : vertices )
    {
      if ( vertex.position.x > maxAABB.x )
        maxAABB.x = vertex.position.x;
      if ( vertex.position.y > maxAABB.y )
        maxAABB.y = vertex.position.y;
      if ( vertex.position.z > maxAABB.z )
        maxAABB.z = vertex.position.z;

      if ( vertex.position.x < minAABB.x )
        minAABB.x = vertex.position.x;
      if ( vertex.position.y < minAABB.y )
        minAABB.y = vertex.position.y;
      if ( vertex.position.z < minAABB.z )
        minAABB.z = vertex.position.z;
    }
  }
  return glm::vec2{ 0, 0 };
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