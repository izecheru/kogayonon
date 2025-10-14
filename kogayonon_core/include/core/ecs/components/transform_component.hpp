#pragma once
#include "utilities/math/math.hpp"

namespace kogayonon_core
{
struct TransformComponent
{
  glm::vec3 translation{ 0.0f };
  glm::vec3 rotation{ 0.0f };
  glm::vec3 scale{ 1.0f };
};
} // namespace kogayonon_core