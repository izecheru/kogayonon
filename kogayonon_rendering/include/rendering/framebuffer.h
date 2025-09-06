#pragma once
#include <memory>

namespace kogayonon_rendering {
class FrameBuffer
{
  public:
    FrameBuffer(int width, int height);
    ~FrameBuffer();

    void bind() const;
    void unbind() const;

    void rescaleFramebuffer(int width, int height);
    unsigned int getTexture() const;
    unsigned int getFbo() const;
    unsigned int getRbo() const;
    int getWidth() const;
    int getHeight() const;

  private:
    int m_width;
    int m_height;
    unsigned int m_rbo;
    unsigned int m_fbo;
    unsigned int m_texture;
};
} // namespace kogayonon_rendering