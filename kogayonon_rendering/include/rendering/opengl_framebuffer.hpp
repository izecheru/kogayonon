#pragma once
#include <memory>
#include "rendering/framebuffer.hpp"

namespace kogayonon_rendering
{
class OpenGLFramebuffer : public Framebuffer
{
public:
  explicit OpenGLFramebuffer( const FramebufferSpec& spec );
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

  void checkFramebuffer() const;

  const FramebufferSpec& getSpecification() override;

  /**
   * @brief Get the color attachment id
   * @param index Index of the attachment
   * @return
   */
  auto getColorAttachmentId( uint32_t index = 0 ) const -> uint32_t override;

  /**
   * @brief Get the depth attachment id
   * @return
   */
  auto getDepthAttachmentId( uint32_t index = 0u ) const -> uint32_t;

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
  auto readPixel( uint32_t attachmentIndex, int x, int y ) const -> int;

  void attachRenderbuffer();

  auto getId() -> uint32_t&;

private:
  void attachColorTexture( uint32_t& id, uint32_t w, uint32_t h, GLenum format, uint32_t& fbo, int index );
  void attachDepthTexture( uint32_t& id, uint32_t w, uint32_t h, GLenum format, GLenum attachmentType, uint32_t& fbo );

private:
  uint32_t m_fbo{ 0 };
  uint32_t m_rbo{ 0 };
  FramebufferSpec m_specification;
};
} // namespace kogayonon_rendering