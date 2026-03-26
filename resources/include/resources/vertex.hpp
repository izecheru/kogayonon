#pragma once
#include <glm/glm.hpp>

namespace resources
{
struct Vertex
{
  glm::vec3 translation;
  glm::vec3 normal;
  glm::vec2 textureCoords;
  glm::ivec4 jointIndices;
  glm::vec4 weights;
};
} // namespace resources