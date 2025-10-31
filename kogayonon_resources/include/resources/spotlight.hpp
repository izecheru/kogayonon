#pragma once
#include <glm/glm.hpp>

namespace kogayonon_resources
{
struct SpotLight
{
  glm::vec4 position{ 0.0f, 0.0f, 0.0f, 1.0f };
  glm::vec4 direction{ 0.0f, 0.0f, -2.0f, 1.0f };
  float cutOff{ glm::cos( glm::radians( 12.5f ) ) };
  float pad[3];
};
} // namespace kogayonon_resources