#pragma once
#include <glad/glad.h>

class Texture
{
public:
  Texture(const char* image_path);
  Texture() {}

  ~Texture() { if (m_id != 0)glDeleteTextures(1, &m_id); }

  void bind();
  void unbind();


private:
  unsigned int m_id;
};
