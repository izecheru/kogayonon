#pragma once
#include <glad/glad.h>

#include <memory>

namespace kogayonon
{
class FrameBuffer
{
public:
  FrameBuffer(int width, int height);
  ~FrameBuffer();

  void bind() const;
  void unbind() const;

  void rescaleFramebuffer(int width, int height);

  inline unsigned int getTexture()
  {
    return m_fbo;
  }

  inline int getWidth()
  {
    return m_width;
  }

  inline int getHeight()
  {
    return m_height;
  }

private:
  int m_width;
  int m_height;
  unsigned int m_rbo;
  unsigned int m_fbo;
  unsigned int m_texture;
};
} // namespace kogayonon