#pragma once
#include <cstdint>

namespace kogayonon_rendering
{
//// preparation to make a more modular frame buffer
// enum class FramebufferTextureFormat
//{
//   None = 0,
//
//   RGBA8,
//   DEPTH24STENCIL8
// };
//
// struct FramebufferTextureSpecification
//{
//   FramebufferTextureSpecification() = default;
//
//   explicit FramebufferTextureSpecification( FramebufferTextureFormat format )
//       : textureFormat{ format }
//   {
//   }
//
//   FramebufferTextureFormat textureFormat;
// };
//
// struct FramebufferAttachmentSpecification
//{
//   FramebufferAttachmentSpecification() = default;
//
//   explicit FramebufferAttachmentSpecification(
//     std::initializer_list<FramebufferTextureSpecification> attachmentSpecification )
//       : attachments{ attachmentSpecification }
//   {
//   }
//
//   std::vector<FramebufferTextureSpecification> attachments;
// };
//
// struct FramebufferSpecification
//{
//   uint32_t width{ 0 };
//   uint32_t height{ 0 };
//   FramebufferAttachmentSpecification attachments;
// };

class FrameBuffer
{
public:
  FrameBuffer( int width, int height );
  ~FrameBuffer();

  void bind() const;
  void unbind() const;

  void rescale( int width, int height );

  uint32_t getTexture() const;
  uint32_t getFBO() const;

  int getWidth() const;
  int getHeight() const;

private:
  int m_width;
  int m_height;
  uint32_t m_fbo;
  uint32_t m_texture;
};
} // namespace kogayonon_rendering