#include "rendering/framebuffer.h"
#include <glad/glad.h>
#include "logger/logger.h"

using namespace kogayonon_logger;

namespace kogayonon_rendering {
FrameBuffer::FrameBuffer( int width, int height ) : m_width( width ), m_height( height )
{
    glCreateFramebuffers( 1, &m_fbo );

    // create color texture
    glCreateTextures( GL_TEXTURE_2D, 1, &m_texture );
    glTextureStorage2D( m_texture, 1, GL_RGBA8, width, height );
    glTextureParameteri( m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTextureParameteri( m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glNamedFramebufferTexture( m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0 );

    // create depth renderbuffer
    glCreateRenderbuffers( 1, &m_rbo );
    glNamedRenderbufferStorage( m_rbo, GL_DEPTH24_STENCIL8, width, height );
    glNamedFramebufferRenderbuffer( m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo );
    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
        Logger::error( "Framebuffer not complete!" );
    }
}

FrameBuffer::~FrameBuffer()
{
    if ( m_texture )
        glDeleteTextures( 1, &m_texture );
    if ( m_fbo )
        glDeleteFramebuffers( 1, &m_fbo );
    if ( m_rbo )
        glDeleteRenderbuffers( 1, &m_rbo );
}

void FrameBuffer::bind() const
{
    glBindFramebuffer( GL_FRAMEBUFFER, m_fbo );
}

void FrameBuffer::unbind() const
{
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void FrameBuffer::rescaleFramebuffer( int width, int height )
{
    m_width = width;
    m_height = height;

    // delete old
    glDeleteTextures( 1, &m_texture );
    glDeleteRenderbuffers( 1, &m_rbo );

    // recreate texture
    glCreateTextures( GL_TEXTURE_2D, 1, &m_texture );
    glTextureStorage2D( m_texture, 1, GL_RGBA8, width, height );
    glTextureParameteri( m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTextureParameteri( m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glNamedFramebufferTexture( m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0 );

    // recreate renderbuffer
    glCreateRenderbuffers( 1, &m_rbo );
    glNamedRenderbufferStorage( m_rbo, GL_DEPTH24_STENCIL8, width, height );
    glNamedFramebufferRenderbuffer( m_fbo, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo );

    if ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) != GL_FRAMEBUFFER_COMPLETE )
    {
        Logger::error( "Framebuffer is not complete after resize!" );
    }
}

unsigned int FrameBuffer::getTexture() const
{
    return m_texture;
}

unsigned int FrameBuffer::getFbo() const
{
    return m_fbo;
}

unsigned int FrameBuffer::getRbo() const
{
    return m_rbo;
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