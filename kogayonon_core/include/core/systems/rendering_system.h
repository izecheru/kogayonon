#pragma once

namespace kogayonon_core
{
// this should instance draw
// glDrawElementsInstanced(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0, modelCount.at(i))
// where modelCount.at(i) is the number of instances of that model we need to draw
class RenderingSystem
{
public:
  RenderingSystem();
  ~RenderingSystem();

private:
};
} // namespace kogayonon_core