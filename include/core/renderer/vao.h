#pragma once
#include <glad/glad.h>

class VertexArrayObject
{
public:
  VertexArrayObject();
  ~VertexArrayObject() { if (m_id != 0)glDeleteBuffers(1, &m_id); }

  void bind();
  void unbind();

  void attribPointer(unsigned int index, int size, unsigned int type, bool normalized, unsigned int stride, const void* offset);
private:
  unsigned int m_id = 0;
};