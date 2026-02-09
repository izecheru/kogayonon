#include "rendering/skeleton_uniformbuffer.hpp"
#include <glad/glad.h>

namespace kogayonon_rendering
{
void SkeletonsUniformbuffer::initialize( uint32_t bindingIndex )
{
  m_bindingIndex = bindingIndex;
  glCreateBuffers( 1, &m_ubo );
  glNamedBufferData( GL_UNIFORM_BUFFER, sizeof( uint32_t ), nullptr, GL_STATIC_DRAW );
  // bind to binding point 0 (must match shader)
  glBindBufferBase( GL_UNIFORM_BUFFER, 0, m_ubo );
}

void SkeletonsUniformbuffer::destroy()
{
  if ( m_ubo )
  {
    glDeleteBuffers( 1, &m_ubo );
    m_ubo = 0;
  }
}

void SkeletonsUniformbuffer::update()
{
  if ( m_ubo == 0 )
    initialize( m_bindingIndex );

  bind();
  // this should get the skeleton thing
  glNamedBufferData( m_ubo, sizeof( uint32_t ), &m_count, GL_DYNAMIC_DRAW );
  glBindBufferBase( GL_UNIFORM_BUFFER, m_bindingIndex, m_ubo );
  unbind();
}

void SkeletonsUniformbuffer::bind()
{
  glBindBuffer( GL_UNIFORM_BUFFER, m_ubo );
}

void SkeletonsUniformbuffer::unbind()
{
  glBindBuffer( GL_UNIFORM_BUFFER, 0 );
}

void SkeletonsUniformbuffer::incrementSkeletonCount()
{
  ++m_count;
}

auto SkeletonsUniformbuffer::getSkeletonCount() const -> uint32_t
{
  return m_count;
}
} // namespace kogayonon_rendering