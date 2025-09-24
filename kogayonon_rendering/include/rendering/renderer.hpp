#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace kogayonon_rendering
{
class Renderer
{
public:
  Renderer();
  ~Renderer();

  void begin();
  void end();

private:
};
} // namespace kogayonon_rendering