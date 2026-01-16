#include "rendering/opengl_framebuffer.hpp"
#include <spdlog/spdlog.h>

namespace kogayonon_rendering
{

void OpenGLFramebuffer::checkFramebuffer() const
{
  auto status = glCheckNamedFramebufferStatus( m_fbo, GL_FRAMEBUFFER );
  if ( status != GL_FRAMEBUFFER_COMPLETE )
  {
    switch ( status )
    {
    case GL_FRAMEBUFFER_UNDEFINED:
      spdlog::error( "FBO undefined." );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
      spdlog::error( "Incomplete attachment." );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
      spdlog::error( "Missing attachment." );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
      spdlog::error( "Incomplete draw buffer." );
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
      spdlog::error( "Incomplete read buffer." );
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
      spdlog::error( "Unsupported framebuffer format." );
      break;
    default:
      spdlog::error( "Framebuffer incomplete: {}", status );
      break;
    }
  }
}

OpenGLFramebuffer::OpenGLFramebuffer( const FramebufferSpec& spec )
    : m_specification{ spec }
    , m_fbo{ 0 }
    , m_rbo{ 0 }
{
  init();
}

void OpenGLFramebuffer::init()
{
  glCreateFramebuffers( 1, &m_fbo );

  auto count = 0u;
  for ( auto& item : m_specification.colorAttachments )
  {
    switch ( item.textureFormat )
    {
    case GL_RGBA8:
    case GL_RGBA:
      attachColorTexture( item.id, m_specification.width, m_specification.height, GL_RGBA8, m_fbo, count++ );
      break;
    case GL_RED_INTEGER:
      attachColorTexture( item.id, m_specification.width, m_specification.height, GL_R32I, m_fbo, count++ );
      break;
    default:
      spdlog::critical( "Texture format unsupported" );
      break;
    }
  }

  for ( auto& item : m_specification.depthAttachments )
  {
    switch ( item.textureFormat )
    {
    case GL_DEPTH_COMPONENT24:
      attachDepthTexture( item.id, m_specification.width, m_specification.height, GL_DEPTH_COMPONENT32,
                          GL_DEPTH_ATTACHMENT, m_fbo );
      break;

    case GL_DEPTH24_STENCIL8:
      attachDepthTexture( item.id, m_specification.width, m_specification.height, GL_DEPTH24_STENCIL8,
                          GL_DEPTH_STENCIL_ATTACHMENT, m_fbo );
      break;
    default:
      spdlog::critical( "Texture format unsupported" );
      break;
    }
  }

  std::vector<GLenum> drawBuffers;
  for ( auto i = 0u; i < m_specification.colorAttachments.size(); ++i )
  {
    drawBuffers.push_back( GL_COLOR_ATTACHMENT0 + static_cast<GLenum>( i ) );
  }

  if ( drawBuffers.empty() )
  {
    glNamedFramebufferDrawBuffer( m_fbo, GL_NONE );
    glNamedFramebufferReadBuffer( m_fbo, GL_NONE );
  }
  else
  {
    glNamedFramebufferDrawBuffers( m_fbo, static_cast<GLsizei>( drawBuffers.size() ), drawBuffers.data() );
  }

  checkFramebuffer();
  glViewport( 0, 0, m_specification.width, m_specification.height );
}

void OpenGLFramebuffer::resize( uint32_t w, uint32_t h )
{
  if ( w == 0 || h == 0 )
    return;

  if ( w == m_specification.width && h == m_specification.height )
    return;

  m_specification.width = w;
  m_specification.height = h;

  destroy();
  init();
}

void OpenGLFramebuffer::destroy()
{
  if ( m_fbo )
  {
    for ( auto i = 0u; i < m_specification.colorAttachments.size(); i++ )
    {
      if ( m_specification.colorAttachments.at( i ).id )
      {
        glDeleteTextures( 1, &m_specification.colorAttachments.at( i ).id );
        m_specification.colorAttachments.at( i ).id = 0;
      }
    }

    for ( auto i = 0u; i < m_specification.depthAttachments.size(); i++ )
    {
      if ( m_specification.depthAttachments.at( i ).id )
      {
        glDeleteTextures( 1, &m_specification.depthAttachments.at( i ).id );
        m_specification.depthAttachments.at( i ).id = 0;
      }
    }

    if ( m_rbo )
      glDeleteRenderbuffers( 1, &m_rbo );

    m_rbo = 0;

    glDeleteFramebuffers( 1, &m_fbo );
    m_fbo = 0;
  }
}

void OpenGLFramebuffer::bind()
{
  glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
  glViewport( 0, 0, m_specification.width, m_specification.height );
}

void OpenGLFramebuffer::unbind()
{
  glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

auto OpenGLFramebuffer::getSpecification() -> const FramebufferSpec&
{
  return m_specification;
}

auto OpenGLFramebuffer::getDepthAttachmentId( uint32_t index ) const -> uint32_t
{
  return m_specification.depthAttachments.at( index ).id;
}

auto OpenGLFramebuffer::getColorAttachmentId( uint32_t index ) const -> uint32_t
{
  return m_specification.colorAttachments.at( index ).id;
}

auto OpenGLFramebuffer::readPixel( uint32_t attachmentIndex, int x, int y ) const -> int
{
  glReadBuffer( GL_COLOR_ATTACHMENT0 + attachmentIndex );
  int flippedY = m_specification.height - y - 1;
  int pixelData = -1;
  glReadPixels( x, flippedY, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData );
  return pixelData;
}

void OpenGLFramebuffer::attachRenderbuffer()
{
  glGenRenderbuffers( 1, &m_rbo );
  glNamedRenderbufferStorage( m_rbo, GL_DEPTH_COMPONENT24, m_specification.width, m_specification.height );
  glNamedFramebufferRenderbuffer( m_fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo );
  std::vector<GLenum> drawBuffers = { GL_COLOR_ATTACHMENT0 };
  glNamedFramebufferDrawBuffers( m_fbo, static_cast<GLsizei>( drawBuffers.size() ), drawBuffers.data() );
}

auto OpenGLFramebuffer::getId() -> uint32_t&
{
  return m_fbo;
}

void OpenGLFramebuffer::clearColorAttachment( uint32_t index, int id ) const
{
  auto& attachment = m_specification.colorAttachments.at( index );

  switch ( attachment.textureFormat )
  {
  case GL_RED_INTEGER:
    glClearTexImage( attachment.id, 0, GL_RED_INTEGER, GL_INT, &id );
    break;
  default:
    float clearColor[4] = { 0.f, 0.f, 0.f, 0.f };
    glClearTexImage( attachment.id, 0, GL_RGBA, GL_FLOAT, clearColor );
    break;
  }
}

void OpenGLFramebuffer::attachColorTexture( uint32_t& id, uint32_t w, uint32_t h, GLenum format, uint32_t& fbo,
                                            int index )

{
  assert( w != 0 && h != 0 && "width and height CANNOT be 0" );
  glCreateTextures( GL_TEXTURE_2D, 1, &id );
  glTextureStorage2D( id, 1, format, w, h );

  glTextureParameteri( id, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
  glTextureParameteri( id, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
  glTextureParameteri( id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
  glTextureParameteri( id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
  glNamedFramebufferTexture( fbo, GL_COLOR_ATTACHMENT0 + index, id, 0 );
}

void OpenGLFramebuffer::attachDepthTexture( uint32_t& id, uint32_t w, uint32_t h, GLenum format, GLenum attachmentType,
                                            uint32_t& fbo )

{
  assert( w != 0 && h != 0 && "width and height CANNOT be 0" );
  glCreateTextures( GL_TEXTURE_2D, 1, &id );
  glBindTexture( GL_TEXTURE_2D, id );
  glTextureStorage2D( id, 1, format, w, h );
  glTextureParameteri( id, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTextureParameteri( id, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTextureParameteri( id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
  glTextureParameteri( id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
  float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  glTextureParameterfv( id, GL_TEXTURE_BORDER_COLOR, borderColor );
  glBindTexture( GL_TEXTURE_2D, 0 );

  glNamedFramebufferTexture( fbo, attachmentType, id, 0 );
}

} // namespace kogayonon_rendering