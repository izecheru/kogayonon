#include "rendering/renderer.hpp"
#include <glad/glad.h>

namespace kogayonon_rendering
{
bool Renderer::isDepthEnabled()
{
  return glIsEnabled( GL_DEPTH_TEST );
}

bool Renderer::isStencilEnabled()
{
  return glIsEnabled( GL_STENCIL_TEST );
}

void Renderer::enableDepth()
{
  if ( isDepthEnabled() )
    return;

  glEnable( GL_DEPTH_TEST );
}

void Renderer::enableStencil()
{
  if ( isStencilEnabled() )
    return;

  glEnable( GL_STENCIL_TEST );
}

void Renderer::disableDepth()
{
  if ( !isDepthEnabled() )
    return;

  glDisable( GL_DEPTH_TEST );
}

void Renderer::disableStencil()
{
  if ( !isStencilEnabled() )
    return;

  glDisable( GL_STENCIL_TEST );
}
} // namespace kogayonon_rendering