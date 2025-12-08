#pragma once
#include <glm/glm.hpp>

namespace kogayonon_resources
{
struct DirectionalLight
{
  glm::vec4 direction{ -0.3f, -1.0f, -0.5f, 0.0f };
  glm::vec4 diffuse{ 1.0f, 1.0f, 1.0f, 1.0f };
  glm::vec4 specular{ 1.0f, 1.0f, 1.0f, 1.0f };
};
} // namespace kogayonon_resources
