#pragma once
#include <cstdint>
#include <glad/glad.h>
#include <initializer_list>
#include <memory>
#include <vector>

namespace kogayonon_rendering
{
enum class FramebufferAttachmentType
{
  Depth,
  Color
};

struct FramebufferAttachment
{
  uint32_t id;
  uint32_t textureFormat;
  FramebufferAttachmentType type;
};

struct FramebufferSpec
{
  FramebufferSpec() = default;

  explicit FramebufferSpec( std::initializer_list<FramebufferAttachment> t_attachments )
      : width{ 800 }
      , height{ 800 }
  {
    for ( auto& attachment : t_attachments )
    {
      if ( attachment.type == FramebufferAttachmentType::Depth )
      {
        depthAttachments.emplace_back( attachment );
      }
      else
      {
        colorAttachments.emplace_back( attachment );
      }
    }
  }

  uint32_t width;
  uint32_t height;
  std::vector<FramebufferAttachment> colorAttachments;
  std::vector<FramebufferAttachment> depthAttachments;
};

class Framebuffer
{
public:
  virtual ~Framebuffer() = default;

  virtual void bind() = 0;
  virtual void unbind() = 0;

  virtual auto getColorAttachmentId( uint32_t index = 0 ) const -> uint32_t = 0;
  virtual void resize( uint32_t w, uint32_t h ) = 0;
  virtual auto getSpecification() -> const FramebufferSpec& = 0;

private:
  FramebufferSpec m_specification;
};
} // namespace kogayonon_rendering