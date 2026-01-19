#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace kogayonon_rendering
{
class Renderer
{
public:
  static bool isDepthEnabled();
  static bool isStencilEnabled();

  static void enableDepth();
  static void enableStencil();
  static void enableColorMask();

  static void disableDepth();
  static void disableStencil();
  static void disableColorMask();

private:
  // copy is not allowed
  Renderer( const Renderer& ) = delete;
  Renderer& operator=( const Renderer& ) = delete;
  Renderer( Renderer&& ) = delete;
  Renderer& operator=( Renderer&& ) = delete;

  // since we use this for kind of like a render command setter, getter and so on
  // we don't need any instances
  Renderer() = delete;
  ~Renderer() = delete;
};
} // namespace kogayonon_rendering