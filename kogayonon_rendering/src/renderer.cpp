#include "rendering/renderer.hpp"
#include <glad/glad.h>

auto kogayonon_rendering::Renderer::getInstance() -> Renderer&
{
  static Renderer renderer{};
  return renderer;
}

bool kogayonon_rendering::Renderer::isDepthEnabled()
{
  return glIsEnabled( GL_DEPTH_TEST );
}

bool kogayonon_rendering::Renderer::isStencilEnabled()
{
  return glIsEnabled( GL_STENCIL_TEST );
}

void kogayonon_rendering::Renderer::enableDepth()
{
  if ( isDepthEnabled() )
    return;

  glEnable( GL_DEPTH_TEST );
}

void kogayonon_rendering::Renderer::enableStencil()
{
  if ( isStencilEnabled() )
    return;

  glEnable( GL_STENCIL_TEST );
}

void kogayonon_rendering::Renderer::disableDepth()
{
  if ( !isDepthEnabled() )
    return;

  glDisable( GL_DEPTH_TEST );
}

void kogayonon_rendering::Renderer::disableStencil()
{
  if ( !isStencilEnabled() )
    return;

  glDisable( GL_STENCIL_TEST );
}
