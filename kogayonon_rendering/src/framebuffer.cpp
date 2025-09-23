#include "rendering/framebuffer.hpp"
#include <glad/glad.h>
#include <spdlog/spdlog.h>

namespace kogayonon_rendering
{
FrameBuffer::FrameBuffer( int width, int height )
    : m_width( width )
    , m_height( height )
{
  glCreateFramebuffers( 1, &m_frameBufferObject );

  // create color texture
  glCreateTextures( GL_TEXTURE_2D, 1, &m_texture );
  glTextureStorage2D( m_texture, 1, GL_RGBA8, width, height );
  glTextureParameteri( m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTextureParameteri( m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glNamedFramebufferTexture( m_frameBufferObject, GL_COLOR_ATTACHMENT0, m_texture, 0 );

  // create depth renderbuffer
  glCreateRenderbuffers( 1, &m_renderBufferObject );
  glNamedRenderbufferStorage( m_renderBufferObject, GL_DEPTH24_STENCIL8, width, height );
  glNamedFramebufferRenderbuffer( m_frameBufferObject, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  m_renderBufferObject );
  if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
  {
    spdlog::error( "Framebuffer not complete!" );
  }
}

FrameBuffer::~FrameBuffer()
{
  if ( m_texture )
    glDeleteTextures( 1, &m_texture );
  if ( m_frameBufferObject )
    glDeleteFramebuffers( 1, &m_frameBufferObject );
  if ( m_renderBufferObject )
    glDeleteRenderbuffers( 1, &m_renderBufferObject );
}

void FrameBuffer::bind() const
{
  glBindFramebuffer( GL_FRAMEBUFFER, m_frameBufferObject );
}

void FrameBuffer::unbind() const
{
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void FrameBuffer::rescale( int width, int height )
{
  m_width = width;
  m_height = height;
  glClearColor( 0.1, 0.1, 0.1, 1.0 );
  glViewport( 0, 0, m_width, m_height );

  // delete old
  glDeleteTextures( 1, &m_texture );
  glDeleteRenderbuffers( 1, &m_renderBufferObject );

  // recreate texture
  glCreateTextures( GL_TEXTURE_2D, 1, &m_texture );
  glTextureStorage2D( m_texture, 1, GL_RGBA8, width, height );
  glTextureParameteri( m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTextureParameteri( m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glNamedFramebufferTexture( m_frameBufferObject, GL_COLOR_ATTACHMENT0, m_texture, 0 );

  // recreate renderbuffer
  glCreateRenderbuffers( 1, &m_renderBufferObject );
  glNamedRenderbufferStorage( m_renderBufferObject, GL_DEPTH24_STENCIL8, width, height );
  glNamedFramebufferRenderbuffer( m_frameBufferObject, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  m_renderBufferObject );

  if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
  {
    spdlog::error( "Framebuffer is not complete after resize!" );
  }
}

unsigned int FrameBuffer::getTexture() const
{
  return m_texture;
}

unsigned int FrameBuffer::getFrameBufferObject() const
{
  return m_frameBufferObject;
}

unsigned int FrameBuffer::getRenderBufferObject() const
{
  return m_renderBufferObject;
}

int FrameBuffer::getHeight() const
{
  return m_height;
}

int FrameBuffer::getWidth() const
{
  return m_width;
}
} // namespace kogayonon_rendering