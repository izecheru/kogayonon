#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Any other GLM includes, including the one triggering the error, go after the define.
// Avoid including 'detail' files manually.
namespace kogayonon_utilities::math
{
static glm::mat4 computeTransform( const glm::vec3& pos, const glm::vec3& rotation, const glm::vec3& scale )
{
  glm::mat4 model{ 1.0f };
  model = glm::translate( model, pos );
  glm::vec3 r = glm::radians( rotation );
  model = glm::rotate( model, r.z, glm::vec3{ 0.0f, 0.0f, 1.0f } ); // roll
  model = glm::rotate( model, r.y, glm::vec3{ 0.0f, 1.0f, 0.0f } ); // yaw
  model = glm::rotate( model, r.x, glm::vec3{ 1.0f, 0.0f, 0.0f } ); // pitch
  model = glm::scale( model, scale );
  return model;
}

static glm::vec3 getRayTracedObject( const glm::vec3& pos, const glm::vec2& mousePos, int w, int h )
{
  float xNormalDevCoord = ( 2 * mousePos.x / w ) - 1;
  float yNormalDevCoord = 1 - ( 2 * mousePos.y / h );
}

static glm::vec3 screenToWorld( const glm::mat4& view, const glm::mat4& projection, const glm::vec2& mousePos,
                                const glm::vec4& viewportSize )
{
  glm::vec3 win{ mousePos.x, viewportSize.x - mousePos.y, 0.0f };
  glm::vec3 point = glm::unProject( win, view, projection, viewportSize );
  return point;
}
} // namespace kogayonon_utilities::math