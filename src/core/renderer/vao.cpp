#include "core/renderer/vao.h"
#include <glad/glad.h>

VertexArrayObject::VertexArrayObject() {
  glGenVertexArrays(1, &m_id);
}

void VertexArrayObject::bind() {
  glBindVertexArray(m_id);
}

void VertexArrayObject::unbind() {
  glBindVertexArray(0);
}

void VertexArrayObject::attribPointer(unsigned int index, int size, unsigned int type, bool normalized, unsigned int stride, const void* offset) {
  glVertexAttribPointer(index, size, type, normalized, stride, offset);
  glEnableVertexAttribArray(index);
}

