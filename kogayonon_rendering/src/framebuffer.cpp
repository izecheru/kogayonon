#include "rendering/framebuffer.hpp"
#include <glad/glad.h>
#include <spdlog/spdlog.h>

namespace kogayonon_rendering
{
FrameBuffer::FrameBuffer( int width, int height )
    : m_width{ width }
    , m_height{ height }
{
  glCreateFramebuffers( 1, &m_fbo );

  // create color texture
  glCreateTextures( GL_TEXTURE_2D, 1, &m_texture );
  glTextureStorage2D( m_texture, 1, GL_RGBA8, width, height );
  glTextureParameteri( m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTextureParameteri( m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glNamedFramebufferTexture( m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0 );

  if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
  {
    spdlog::error( "Framebuffer not complete!" );
  }
}

FrameBuffer::~FrameBuffer()
{
  if ( m_fbo )
  {
    glDeleteTextures( 1, &m_texture );
    glDeleteFramebuffers( 1, &m_fbo );
  }
}

void FrameBuffer::bind() const
{
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
}

void FrameBuffer::unbind() const
{
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void FrameBuffer::rescale( int width, int height )
{
  m_width = width;
  m_height = height;
  glViewport( 0, 0, m_width, m_height );

  // delete old
  glDeleteTextures( 1, &m_texture );

  // recreate texture
  glCreateTextures( GL_TEXTURE_2D, 1, &m_texture );
  glTextureStorage2D( m_texture, 1, GL_RGBA8, width, height );
  glTextureParameteri( m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTextureParameteri( m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glNamedFramebufferTexture( m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0 );

  if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
  {
    spdlog::error( "Frame buffer is not complete after resize!" );
  }
}

uint32_t FrameBuffer::getTexture() const
{
  return m_texture;
}

uint32_t FrameBuffer::getFBO() const
{
  return m_fbo;
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