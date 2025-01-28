
#include "core/renderer/texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stbi/stb_image.h"
#include "core/logger.h"

Texture::Texture(const char* image_path) {
  int w_image, h_image, num_col_ch;

  // Flip image vertically for correct orientation
  stbi_set_flip_vertically_on_load(true);

  // Generate texture
  glGenTextures(1, &m_id);
  bind();

  // Set texture parameters (wrap and filter modes)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // Load image
  unsigned char* image_data = stbi_load(image_path, &w_image, &h_image, &num_col_ch, 0);
  if (!image_data)
  {
    Logger::logError("Failed to load texture from:", image_path);
    return; // Exit if image loading fails
  }

  Logger::logInfo("Successfully loaded texture from:", image_path);

  // Determine format based on the number of channels
  GLenum format = (num_col_ch == 3) ? GL_RGB : GL_RGBA;

  // Upload image to OpenGL
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w_image, h_image, 0, format, GL_UNSIGNED_BYTE, image_data);
  glGenerateMipmap(GL_TEXTURE_2D);

  // Free the loaded image memory
  stbi_image_free(image_data);

  unbind(); // Unbind texture
}

void Texture::bind() {
  glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::unbind() {
  glBindTexture(GL_TEXTURE_2D, 0);
}
