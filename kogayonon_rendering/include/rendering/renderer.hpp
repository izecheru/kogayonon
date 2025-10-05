#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace kogayonon_rendering
{
struct Vertex_
{
  glm::vec3 pos; // Position
  glm::vec2 textureCoordinates;
  glm::vec4 color;
};

struct Quad
{
  std::vector<Vertex_> vertices{};
  std::vector<uint32_t> indices{
    0, 1, 2, // First triangle
    2, 3, 0  // Second triangle
  };
};

class Renderer
{
public:
  Renderer();
  ~Renderer();

  void drawQuad();

private:
};
} // namespace kogayonon_rendering