#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Any other GLM includes, including the one triggering the error, go after the define.
// Avoid including 'detail' files manually.
namespace kogayonon_utilities::math
{
static glm::mat4 computeModelMatrix( const glm::vec3& pos, const glm::vec3& rotation, const glm::vec3 scale )
{
  glm::mat4 model{ 1.0f };
  model = glm::translate( model, pos );
  model = ::glm::rotate( model, rotation.x, glm::vec3{ 0.0f, 0.0f, 1.0f } );
  model = ::glm::rotate( model, rotation.y, glm::vec3{ 0.0f, 1.0f, 0.0f } );
  model = ::glm::rotate( model, rotation.z, glm::vec3{ 1.0f, 0.0f, 0.0f } );
  model = glm::scale( model, scale );
  return model;
}
} // namespace kogayonon_utilities::math