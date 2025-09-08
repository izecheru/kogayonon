#pragma once

namespace kogayonon_rendering {
class FrameBuffer
{
  public:
    FrameBuffer( int width, int height );
    ~FrameBuffer();

    void bind() const;
    void unbind() const;

    void rescale( int width, int height );

    unsigned int getTexture() const;
    unsigned int getFrameBufferObject() const;
    unsigned int getRenderBufferObject() const;

    int getWidth() const;
    int getHeight() const;

  private:
    int m_width;
    int m_height;
    unsigned int m_renderBufferObject;
    unsigned int m_frameBufferObject;
    unsigned int m_texture;
};
} // namespace kogayonon_rendering