#pragma once
#include <cstdint>
#include <glad/glad.h>
#include <initializer_list>
#include <memory>
#include <vector>

namespace kogayonon_rendering
{
/**
 * @brief Enum for texture formats,
 */
enum class FramebufferTextureFormat
{
  None = 0,

  RGBA8,
  RED_INTEGER,
  DEPTH,
  DEPTH24STENCIL8,

  Depth = DEPTH24STENCIL8
};

static GLenum FramebufferTextureFormatToOpenGL( FramebufferTextureFormat format )
{
  switch ( format )
  {
  case FramebufferTextureFormat::DEPTH24STENCIL8:
    return GL_DEPTH24_STENCIL8;
  case FramebufferTextureFormat::RGBA8:
    return GL_RGBA8;
  case FramebufferTextureFormat::RED_INTEGER:
    return GL_RED_INTEGER;
  }
}

struct FramebufferAttachment
{
  FramebufferAttachment()
  {
  }

  explicit FramebufferAttachment( FramebufferTextureFormat t_format )
      : id{ 0 }
      , textureFormat{ t_format }

  {
  }

  uint32_t id;
  FramebufferTextureFormat textureFormat;
};

struct FramebufferSpecification
{
  FramebufferSpecification() = default;

  explicit FramebufferSpecification( std::initializer_list<FramebufferAttachment> t_attachments, uint32_t w,
                                     uint32_t h )
      : width{ w }
      , height{ h }
      , attachments{ t_attachments }
  {
  }

  explicit FramebufferSpecification( std::initializer_list<FramebufferTextureFormat> t_formats, uint32_t w, uint32_t h )
      : width{ w }
      , height{ h }
  {
    for ( auto format : t_formats )
      attachments.emplace_back( format );
  }

  uint32_t width{ 0 };
  uint32_t height{ 0 };
  std::vector<FramebufferAttachment> attachments;
};

class Framebuffer
{
public:
  virtual ~Framebuffer() = default;

  virtual void bind() = 0;
  virtual void unbind() = 0;

  virtual uint32_t getColorAttachmentId( uint32_t index = 0 ) const = 0;
  virtual void resize( uint32_t w, uint32_t h ) = 0;
  virtual const FramebufferSpecification& getSpecification() = 0;

private:
  FramebufferSpecification m_specification;
};
} // namespace kogayonon_rendering