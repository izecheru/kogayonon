#include "rendering/opengl_framebuffer.hpp"
#include <spdlog/spdlog.h>

namespace kogayonon_rendering
{
namespace utils
{

static GLenum textureFormatToOpenglInternal( FramebufferTextureFormat format )
{
  switch ( format )
  {
  case FramebufferTextureFormat::RGBA8:
    return GL_RGBA8; // internal storage
  case FramebufferTextureFormat::RED_INTEGER:
    return GL_R32UI;
  case FramebufferTextureFormat::DEPTH24STENCIL8:
    return GL_DEPTH24_STENCIL8;
  default:
    return GL_RGBA8;
  }
}

// base format for glReadPixels / glClearTexImage 'format' param
static GLenum textureFormatToBaseFormat( FramebufferTextureFormat format )
{
  switch ( format )
  {
  case FramebufferTextureFormat::RGBA8:
    return GL_RGBA; // base format for RGBA8
  case FramebufferTextureFormat::RED_INTEGER:
    return GL_RED_INTEGER; // integer red
  case FramebufferTextureFormat::DEPTH24STENCIL8:
    return GL_DEPTH_STENCIL;
  default:
    return GL_RGBA;
  }
}

// proper type for glReadPixels / glClearTexImage 'type' param
static GLenum textureFormatToType( FramebufferTextureFormat format )
{
  switch ( format )
  {
  case FramebufferTextureFormat::RGBA8:
    return GL_UNSIGNED_BYTE;
  case FramebufferTextureFormat::RED_INTEGER:
    return GL_UNSIGNED_INT;
  case FramebufferTextureFormat::DEPTH24STENCIL8:
    return GL_UNSIGNED_INT_24_8;
  default:
    return GL_UNSIGNED_BYTE;
  }
}

static void createTexture( uint32_t* id, uint32_t count )
{
  glCreateTextures( GL_TEXTURE_2D, count, id );
}

static void linkFramebufferColorTexture( uint32_t attachmentIndex, uint32_t id, uint32_t fbo )
{
  glNamedFramebufferTexture( fbo, GL_COLOR_ATTACHMENT0 + attachmentIndex, id, 0 );
}

static void linkFramebufferDepthTexture( GLenum attachmentType, uint32_t id, uint32_t fbo )
{
  glNamedFramebufferTexture( fbo, attachmentType, id, 0 );
}

static void attachColorTexture( uint32_t id, uint32_t w, uint32_t h, GLenum format, uint32_t fbo, int index )

{
  assert( w != 0 && h != 0 && "width and height CANNOT be 0" );
  glTextureStorage2D( id, 1, format, w, h );

  glTextureParameteri( id, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTextureParameteri( id, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTextureParameteri( id, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTextureParameteri( id, GL_TEXTURE_WRAP_T, GL_REPEAT );
  glNamedFramebufferTexture( fbo, GL_COLOR_ATTACHMENT0 + index, id, 0 );
}

static void attachDepthTexture( uint32_t id, uint32_t w, uint32_t h, GLenum format, GLenum attachmentType,
                                uint32_t fbo )

{
  assert( w != 0 && h != 0 && "width and height CANNOT be 0" );

  glTextureStorage2D( id, 1, format, w, h );

  glTextureParameteri( id, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
  glTextureParameteri( id, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
  glTextureParameteri( id, GL_TEXTURE_WRAP_S, GL_REPEAT );
  glTextureParameteri( id, GL_TEXTURE_WRAP_T, GL_REPEAT );

  glNamedFramebufferTexture( fbo, attachmentType, id, 0 );
}

static void checkFramebuffer( uint32_t fbo )
{
  auto status = glCheckNamedFramebufferStatus( fbo, GL_FRAMEBUFFER );
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
} // namespace utils

OpenGLFramebuffer::OpenGLFramebuffer( const FramebufferSpecification& spec )
    : m_specification{ spec }
{
  init();
}

void OpenGLFramebuffer::init()
{
  glCreateFramebuffers( 1, &m_fbo );
  for ( int i = 0; i < m_specification.attachments.size(); i++ )
  {
    auto& attachment = m_specification.attachments.at( i );
    utils::createTexture( &attachment.id, 1 );
    switch ( attachment.textureFormat )
    {
    case FramebufferTextureFormat::RGBA8:
      utils::attachColorTexture( attachment.id, m_specification.width, m_specification.height, GL_RGBA8, m_fbo, i );
      break;
    case FramebufferTextureFormat::RED_INTEGER:
      utils::attachColorTexture( attachment.id, m_specification.width, m_specification.height, GL_R32UI, m_fbo, i );
      break;
    case FramebufferTextureFormat::DEPTH:
      utils::attachDepthTexture( attachment.id, m_specification.width, m_specification.height, GL_DEPTH24_STENCIL8,
                                 GL_DEPTH_ATTACHMENT, m_fbo );
      break;
    default:
      break;
    }
  }
  std::vector<GLenum> drawBuffers;
  for ( size_t i = 0; i < m_specification.attachments.size(); ++i )
  {
    if ( m_specification.attachments.at( i ).textureFormat != FramebufferTextureFormat::DEPTH24STENCIL8 )
      drawBuffers.push_back( GL_COLOR_ATTACHMENT0 + static_cast<GLenum>( i ) );
  }

  if ( drawBuffers.empty() )
  {
    glNamedFramebufferDrawBuffer( m_fbo, GL_NONE );
  }
  else
  {
    glNamedFramebufferDrawBuffers( m_fbo, static_cast<GLsizei>( drawBuffers.size() ), drawBuffers.data() );
  }
  utils::checkFramebuffer( m_fbo );
  glViewport( 0, 0, m_specification.width, m_specification.height );
}

void OpenGLFramebuffer::resize( uint32_t w, uint32_t h )
{
  if ( w == m_specification.width || h == m_specification.height )
    return;

  destroy();
  m_specification.width = w;
  m_specification.height = h;
  init();
}

void OpenGLFramebuffer::destroy()
{
  if ( m_fbo )
  {
    for ( int i = 0; i < m_specification.attachments.size(); i++ )
    {
      if ( m_specification.attachments.at( i ).id )
      {
        glDeleteTextures( 1, &m_specification.attachments.at( i ).id );
        m_specification.attachments.at( i ).id = 0;
      }
    }

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

const FramebufferSpecification& OpenGLFramebuffer::getSpecification()
{
  return m_specification;
}

uint32_t OpenGLFramebuffer::getColorAttachmentId( uint32_t index ) const
{
  return m_specification.attachments.at( index ).id;
}

int OpenGLFramebuffer::readPixel( uint32_t attachmentIndex, int x, int y )
{
  assert( m_specification.attachments.size() > attachmentIndex && "index out of bounds in read pixel" );
  auto& attachment = m_specification.attachments.at( attachmentIndex );

  bind();
  glReadBuffer( GL_COLOR_ATTACHMENT0 + attachmentIndex );
  int flippedY = m_specification.height - y - 1;

  int pixelData = -1;

  GLenum baseFormat = utils::textureFormatToBaseFormat( attachment.textureFormat );
  GLenum type = utils::textureFormatToType( attachment.textureFormat );

  if ( attachment.textureFormat == FramebufferTextureFormat::RED_INTEGER )
  {
    glReadPixels( x, flippedY, 1, 1, baseFormat, type, &pixelData );
  }
  else if ( attachment.textureFormat == FramebufferTextureFormat::RGBA8 )
  {
    GLubyte rgba[4] = { 0, 0, 0, 0 };
    glReadPixels( x, flippedY, 1, 1, baseFormat, type, rgba );
    pixelData = rgba[0] | ( rgba[1] << 8 ) | ( rgba[2] << 16 ) | ( rgba[3] << 24 );
  }
  else
  {
    // fallback: try generic read (but prefer explicit cases)
    glReadPixels( x, flippedY, 1, 1, baseFormat, type, &pixelData );
  }

  glReadBuffer( GL_NONE );
  unbind();
  return pixelData;
}

void OpenGLFramebuffer::clearColorAttachment( uint32_t index, int value ) const
{
  auto& attachment = m_specification.attachments.at( index );

  GLenum format = GL_RGBA;
  GLenum type = GL_UNSIGNED_INT;

  if ( attachment.textureFormat == FramebufferTextureFormat::RED_INTEGER )
  {
    format = GL_RED_INTEGER;
    type = GL_UNSIGNED_INT;
  }
  else if ( attachment.textureFormat == FramebufferTextureFormat::RGBA8 )
  {
    format = GL_RGBA;
    type = GL_UNSIGNED_BYTE;
  }

  glClearTexImage( attachment.id, 0, format, type, &value );
}
} // namespace kogayonon_rendering