#include "rendering/lightcount_uniformbuffer.hpp"
#include <glad/glad.h>

namespace kogayonon_rendering
{

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

auto LightCountUniformbuffer::getLightCount( const kogayonon_resources::LightType& type ) const -> uint32_t
{
  switch ( type )
  {
  case kogayonon_resources::LightType::Point: {
    return m_count.numPointLights;
  }
  case kogayonon_resources::LightType::Directional: {
    return m_count.numDirectionalLigths;
  }
  case kogayonon_resources::LightType::Spot: {
    return m_count.numSpotLigths;
  }
  }
}

void LightCountUniformbuffer::incrementLightCount( const kogayonon_resources::LightType& type )
{
  switch ( type )
  {
  case kogayonon_resources::LightType::Point: {
    ++m_count.numPointLights;
    break;
  }
  case kogayonon_resources::LightType::Directional: {
    ++m_count.numDirectionalLigths;
    break;
  }
  case kogayonon_resources::LightType::Spot: {
    ++m_count.numSpotLigths;
    break;
  }
  }
}

void LightCountUniformbuffer::decrementLightCount( const kogayonon_resources::LightType& type )
{

  switch ( type )
  {
  case kogayonon_resources::LightType::Point: {
    --m_count.numPointLights;
    break;
  }
  case kogayonon_resources::LightType::Directional: {
    --m_count.numDirectionalLigths;
    break;
  }
  case kogayonon_resources::LightType::Spot: {
    --m_count.numSpotLigths;
    break;
  }
  }
}

} // namespace kogayonon_rendering