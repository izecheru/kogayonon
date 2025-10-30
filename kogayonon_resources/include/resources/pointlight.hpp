#pragma once
#include <glm/glm.hpp>

namespace kogayonon_resources
{
struct PointLight
{
  glm::vec4 position{ 0.0f, 0.0f, 0.0f, 1.0f }; // default at origin
  glm::vec4 ambient{ 0.1f, 0.1f, 0.1f, 1.0f };  // subtle ambient
  glm::vec4 diffuse{ 1.0f, 1.0f, 1.0f, 1.0f };  // full white diffuse
  glm::vec4 specular{ 1.0f, 1.0f, 1.0f, 1.0f }; // white highlights
  glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
  glm::vec4 params{ 1.0f, 0.09f, 0.032f, 1.0f };
};

static_assert( sizeof( PointLight ) % 16 == 0, "must be multiple of 16" );
} // namespace kogayonon_resources