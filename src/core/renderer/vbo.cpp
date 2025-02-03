#include "core/renderer/vbo.h"
#include <glad/glad.h>

VertexBufferObject::VertexBufferObject(const std::vector<Vertex>& vertices) {
  glGenBuffers(1, &m_id);
  bind();
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

VertexBufferObject::VertexBufferObject(VertexBufferObject&& other) noexcept {
  m_id = other.m_id;
  other.m_id = 0;
}

VertexBufferObject& VertexBufferObject::operator=(VertexBufferObject&& other) noexcept {
  if (this != &other)
  {
    glDeleteBuffers(1, &m_id);
    m_id = other.m_id;
    other.m_id = 0;
  }
  return *this;
}

void VertexBufferObject::bind() {
  glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBufferObject::unbind() {
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
