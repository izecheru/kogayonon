#pragma once
#include <glm/ext/matrix_transform.inl>
#include <glm/glm.hpp>

namespace kogayonon_core
{
struct TransformComponent
{
  TransformComponent()
  {
    // translate
    modelMatrix = glm::translate( modelMatrix, pos );

    // rotate (Z * Y * X)
    modelMatrix = glm::rotate( modelMatrix, rotation.z, glm::vec3( 0, 0, 1 ) );
    modelMatrix = glm::rotate( modelMatrix, rotation.y, glm::vec3( 0, 1, 0 ) );
    modelMatrix = glm::rotate( modelMatrix, rotation.x, glm::vec3( 1, 0, 0 ) );

    // scale
    modelMatrix = glm::scale( modelMatrix, scale );
  }

  /**
   * @brief Will update the model matrix, must be called if we edit the positoin, scale or rotation of the object
   */
  void updateMatrix()
  {
    modelMatrix = glm::mat4{ 1.0f };

    modelMatrix = glm::translate( modelMatrix, pos );

    // rotate (Z * Y * X)
    modelMatrix = glm::rotate( modelMatrix, rotation.z, glm::vec3( 0, 0, 1 ) );
    modelMatrix = glm::rotate( modelMatrix, rotation.y, glm::vec3( 0, 1, 0 ) );
    modelMatrix = glm::rotate( modelMatrix, rotation.x, glm::vec3( 1, 0, 0 ) );

    // scale
    modelMatrix = glm::scale( modelMatrix, scale );
  }

  glm::vec3 pos{ 0.0f };
  glm::vec3 rotation{ 0.0f };
  glm::vec3 scale{ 1.0f };
  glm::mat4 modelMatrix{ 1.0f };
};
} // namespace kogayonon_core