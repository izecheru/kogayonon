#include "rendering/lightcount_uniformbuffer.hpp"
#include <glad/glad.h>

namespace kogayonon_rendering
{
void LightCountUniformbuffer::incrementPointLights()
{
  ++m_count.numPointLights;
}

void LightCountUniformbuffer::incrementDirectionalLights()
{
  ++m_count.numDirectionalLigths;
}

void LightCountUniformbuffer::decrementPointLights()
{
  --m_count.numPointLights;
}

void LightCountUniformbuffer::decrementDirectionalLights()
{
  --m_count.numDirectionalLigths;
}

void LightCountUniformbuffer::initialize( uint32_t bindingIndex )
{
  m_bindingIndex = bindingIndex;
  glCreateBuffers( 1, &m_ubo );
  glNamedBufferData( GL_UNIFORM_BUFFER, sizeof( LightCount ), nullptr, GL_STATIC_DRAW );
  // bind to binding point 0 (must match shader)
  glBindBufferBase( GL_UNIFORM_BUFFER, 0, m_ubo );
}

void LightCountUniformbuffer::destroy()
{
  if ( m_ubo )
  {
    glDeleteBuffers( 1, &m_ubo );
    m_ubo = 0;
  }
}

void LightCountUniformbuffer::update()
{
  if ( m_ubo == 0 )
    initialize( m_bindingIndex );

  bind();
  glNamedBufferData( m_ubo, sizeof( LightCount ), &m_count, GL_DYNAMIC_DRAW );
  glBindBufferBase( GL_UNIFORM_BUFFER, m_bindingIndex, m_ubo );
  unbind();
}

void LightCountUniformbuffer::bind()
{
  glBindBuffer( GL_UNIFORM_BUFFER, m_ubo );
}

void LightCountUniformbuffer::unbind()
{
  glBindBuffer( GL_UNIFORM_BUFFER, 0 );
}

uint32_t LightCountUniformbuffer::getPointLightCount() const
{
  return m_count.numPointLights;
}

uint32_t LightCountUniformbuffer::getDirectionalLightCount() const
{
  return m_count.numDirectionalLigths;
}

} // namespace kogayonon_rendering