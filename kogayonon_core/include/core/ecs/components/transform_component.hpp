#pragma once
#include "utilities/math/math.hpp"

namespace kogayonon_core
{
struct TransformComponent
{
  glm::vec3 pos{ 0.0f };
  glm::vec3 rotation{ 0.0f };
  glm::vec3 scale{ 1.0f };

  glm::mat4 modelMatrix{ 1.0f };

  void updateMatrix()
  {
    modelMatrix = kogayonon_utilities::math::computeModelMatrix( pos, rotation, scale );
  }
};
} // namespace kogayonon_core