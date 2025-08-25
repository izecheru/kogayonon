#include "framebuffer.h"

#include "context_manager/context_manager.h"

namespace kogayonon
{
FrameBuffer::FrameBuffer()
{
  glCreateFramebuffers(1, &m_fbo);

  // Create color texture
  glCreateTextures(GL_TEXTURE_2D, 1, &m_tex);
  glTextureStorage2D(m_tex, 1, GL_RGBA8, m_width, m_height);
  glTextureParameteri(m_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTextureParameteri(m_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glNamedFramebufferTexture(m_fbo, GL_COLOR_ATTACHMENT0, m_tex, 0);

  // Create depth texture
  glCreateTextures(GL_TEXTURE_2D, 1, &m_depth_tex);
  glTextureStorage2D(m_depth_tex, 1, GL_DEPTH24_STENCIL8, m_width, m_height);
  glTextureParameteri(m_depth_tex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTextureParameteri(m_depth_tex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glNamedFramebufferTexture(m_fbo, GL_DEPTH_ATTACHMENT, m_depth_tex, 0);

  glCreateRenderbuffers(1, &m_rbo);
  glNamedRenderbufferStorage(m_rbo, GL_DEPTH24_STENCIL8, m_width, m_height);
  glNamedFramebufferRenderbuffer(m_fbo, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);

  if (glCheckNamedFramebufferStatus(m_fbo, GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    ContextManager::klogger()->error("Framebuffer error");
  }
}

FrameBuffer::~FrameBuffer()
{
  if (m_tex)
    glDeleteTextures(1, &m_tex);
  if (m_depth_tex)
    glDeleteTextures(1, &m_depth_tex);
  if (m_fbo)
    glDeleteFramebuffers(1, &m_fbo);
}

void FrameBuffer::bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void FrameBuffer::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
} // namespace kogayonon