#pragma once
#include <glad/glad.h>

#include <memory>

#include "ui/imgui_window.h"

namespace kogayonon
{
class FrameBuffer
{
public:
  FrameBuffer();
  ~FrameBuffer();

  void bind();
  void unbind();

  inline unsigned int& FBO()
  {
    return m_fbo;
  }

  inline unsigned int& TEX()
  {
    return m_fbo;
  }

  inline void setViewport(double w, double h)
  {
    m_width = w;
    m_height = h;

    // Resize color texture
    glTextureStorage2D(m_tex, 1, GL_RGBA8, w, h);

    // Resize depth renderbuffer
    glNamedRenderbufferStorage(m_rbo, GL_DEPTH24_STENCIL8, w, h);
  }

  inline double getWidth() const
  {
    return m_width;
  }

  inline double getHeight() const
  {
    return m_height;
  }

private:
  double m_width = 0;
  double m_height = 0;

private:
  unsigned int m_rbo;
  unsigned int m_fbo;
  unsigned int m_tex;
  unsigned int m_depth_tex;
};
} // namespace kogayonon