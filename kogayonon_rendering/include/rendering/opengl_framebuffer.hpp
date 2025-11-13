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

  /**
   * @brief Resize the framebuffer, this destroys and reinitializes the buffer
   * @param w -width
   * @param h -height
   */
  void resize( uint32_t w, uint32_t h ) override;

  const FramebufferSpecification& getSpecification() override;

  /**
   * @brief Get the color attachment id
   * @param index Index of the attachment
   * @return
   */
  uint32_t getColorAttachmentId( uint32_t index = 0 ) const override;

  /**
   * @brief Get the depth attachment id
   * @return
   */
  uint32_t getDepthAttachmentId() const;

  /**
   * @brief Clears an attachment from the framebuffer
   * @param index Index of the attachment in the spec attachment array
   * @param id The id of the attachment
   */
  void clearColorAttachment( uint32_t index, int id ) const;

  /**
   * @brief Read the value from the fragment shader at a specific coordinate
   * @param attachmentIndex The attachment index which we use to retrieve a framebuffer texture to read from
   * @param x
   * @param y
   * @return The value read from the fragment shader
   */
  int readPixel( uint32_t attachmentIndex, int x, int y );

  void attachRenderbuffer();

  void bindTexture( uint32_t index );
  uint32_t& getId();

private:
  uint32_t m_fbo{ 0 };
  uint32_t m_rbo{ 0 };
  FramebufferSpecification m_specification;
};
} // namespace kogayonon_rendering