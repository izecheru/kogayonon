#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace kogayonon_rendering
{
class Renderer
{
public:
  static auto getInstance() -> Renderer&;

  static bool isDepthEnabled();
  static bool isStencilEnabled();

  static void enableDepth();
  static void enableStencil();

  static void disableDepth();
  static void disableStencil();

private:
  // copy is not allowed
  Renderer( const Renderer& ) = delete;
  Renderer& operator=( const Renderer& ) = delete;
  Renderer( Renderer&& ) = delete;
  Renderer& operator=( Renderer&& ) = delete;

  Renderer() = default;
  ~Renderer() = default;
};
} // namespace kogayonon_rendering