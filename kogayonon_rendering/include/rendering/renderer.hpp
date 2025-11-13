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

  void clearColor( float r, float g, float b );
  void clearColor( float r, float g, float b, float alpha );
  void clearDepth();
  void clearColor();
  void clearDepthAndColor();

  void bindTexture( uint32_t id );
  void bindBuffer( uint32_t id );

  void destroyBuffer( uint32_t id );
};
} // namespace kogayonon_rendering