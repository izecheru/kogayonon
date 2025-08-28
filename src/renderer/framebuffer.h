#pragma once

#include <glad/glad.h>

#include <memory>

namespace kogayonon
{
/// <summary>
/// This is a class to manage OpenGL frame buffer
/// </summary>
class FrameBuffer
{
public:
  FrameBuffer(int width, int height);
  ~FrameBuffer();

  void bind() const;
  void unbind() const;

  /// <summary>
  /// rescales the frame buffer, deletes the texture and render buffer upon rescale
  /// </summary>
  /// <param name="width"></param>
  /// <param name="height"></param>
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