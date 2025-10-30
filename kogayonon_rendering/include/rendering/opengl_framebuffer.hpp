#pragma once
#include <memory>
#include "rendering/framebuffer.hpp"

namespace kogayonon_rendering
{
class OpenGLFramebuffer : public Framebuffer
{
public:
  explicit OpenGLFramebuffer( const FramebufferSpecification& spec );
  OpenGLFramebuffer() = default;

  void bind() override;
  void unbind() override;

  void init();
  void destroy();
  void resize( uint32_t w, uint32_t h ) override;
  const FramebufferSpecification& getSpecification() override;
  uint32_t getColorAttachmentId( uint32_t index = 0 ) const override;
  uint32_t getDepthAttachmentId() const;
  void clearColorAttachment( uint32_t index, int value ) const;
  int readPixel( uint32_t attachmentIndex, int x, int y );

private:
  uint32_t m_fbo{ 0 };
  FramebufferSpecification m_specification;
};
} // namespace kogayonon_rendering