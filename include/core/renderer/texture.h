#pragma once
#include <glad/glad.h>
#include "shader/shader.h"
#include <assimp/types.h>

class Texture
{
public:
  Texture(const char* image_path, const char* texture_type, unsigned int slot);
  Texture() {}

  ~Texture() { if (m_id != 0)glDeleteTextures(1, &m_id); }

  void bind();
  void unbind();

  const char* getType()const;
  unsigned int getId()const;
  unsigned int getUnit()const;

private:
  unsigned int m_id;
  aiString m_path;
  const char* m_type;
  unsigned int m_unit;
};
