#pragma once
#include <cstdint>

namespace kogayonon_rendering
{
class FrameBuffer
{
public:
  FrameBuffer( int width, int height );
  ~FrameBuffer();

  void bind() const;
  void unbind() const;

  void rescale( int width, int height );

  uint32_t getTexture() const;
  uint32_t getFrameBufferObject() const;
  uint32_t getRenderBufferObject() const;

  int getWidth() const;
  int getHeight() const;

private:
  int m_width;
  int m_height;
  uint32_t m_renderBufferObject;
  uint32_t m_frameBufferObject;
  uint32_t m_texture;
};
} // namespace kogayonon_rendering