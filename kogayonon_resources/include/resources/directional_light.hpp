#pragma once
#include <glm/glm.hpp>

namespace kogayonon_resources
{
struct DirectionalLight
{
  // Direction toward the ground and slightly angled
  glm::vec4 direction{ -0.3f, -1.0f, -0.5f, 0.0f };

  // Warm ambient light to simulate sky fill
  glm::vec4 ambient{ 0.3f, 0.25f, 0.2f, 1.0f };

  // Strong, warm diffuse light for sunlight
  glm::vec4 diffuse{ 1.0f, 0.95f, 0.8f, 1.0f };

  // Bright specular highlights, slightly yellowish-white
  glm::vec4 specular{ 1.0f, 0.97f, 0.9f, 1.0f };
};
} // namespace kogayonon_resources
