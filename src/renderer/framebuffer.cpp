#include "framebuffer.h"

#include "context_manager/context_manager.h"

namespace kogayonon
{
FrameBuffer::FrameBuffer(int width, int height) : m_width(width), m_height(height)
{
  glCreateFramebuffers(1, &m_fbo);

  // Create color texture
  glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
  glTextureStorage2D(m_texture, 1, GL_RGBA8, width, height);
  glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);

  // Create depth renderbuffer
  glCreateRenderbuffers(1, &m_rbo);
  glNamedRenderbufferStorage(m_rbo, GL_DEPTH24_STENCIL8, width, height);
}

FrameBuffer::~FrameBuffer()
{
  if (m_texture)
    glDeleteTextures(1, &m_texture);
  if (m_fbo)
    glDeleteFramebuffers(1, &m_fbo);
  if (m_rbo)
    glDeleteRenderbuffers(1, &m_rbo);
}

void FrameBuffer::bind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void FrameBuffer::unbind() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::rescaleFramebuffer(int width, int height)
{
  m_width = width;
  m_height = height;

  // delete old
  glDeleteTextures(1, &m_texture);
  glDeleteRenderbuffers(1, &m_rbo);

  // recreate texture
  glCreateTextures(GL_TEXTURE_2D, 1, &m_texture);
  glTextureStorage2D(m_texture, 1, GL_RGBA8, width, height);
  glTextureParameteri(m_texture, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(m_texture, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_texture, 0);

  // recreate renderbuffer
  glCreateRenderbuffers(1, &m_rbo);
  glNamedRenderbufferStorage(m_rbo, GL_DEPTH24_STENCIL8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    KLogger::error("ERROR: Framebuffer is not complete after resize!");
}
} // namespace kogayonon