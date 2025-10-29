#pragma once
#include <glm/glm.hpp>

namespace kogayonon_resources
{
struct PointLight
{
  glm::vec4 position = glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f }; // default at origin
  glm::vec4 ambient = glm::vec4{ 0.1f, 0.1f, 0.1f, 1.0f };  // subtle ambient
  glm::vec4 diffuse = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f };  // full white diffuse
  glm::vec4 specular = glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f }; // white highlights

  float constant{ 1.0f };
  float linear{ 0.09f };
  float quadratic{ 0.032f };

  // Padding to align to 16 bytes (optional, depends on std140 layout)
  float pad{ 0.0f };
};
} // namespace kogayonon_resources