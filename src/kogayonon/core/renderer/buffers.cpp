#include <core/renderer/buffer.h>
#include <glad/glad.h>

VertexBufferObject::VertexBufferObject(const std::vector<Vertex>& vertices) {
  glGenBuffers(1, &m_id);
  bind();
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
}

void VertexBufferObject::bind() {
  glBindBuffer(GL_ARRAY_BUFFER, m_id);
}

void VertexBufferObject::unbind() {
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ElementsBufferObject::ElementsBufferObject(const std::vector<unsigned int>& indices) {
  this->indices = indices;
  glGenBuffers(1, &m_id);
  bind();
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
}

void ElementsBufferObject::bind() {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
}

void ElementsBufferObject::unbind() {
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

const std::vector<unsigned int> ElementsBufferObject::getIndices() const {
  return indices;
}


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
  glEnableVertexAttribArray(index);
  glVertexAttribPointer(index, size, type, normalized, stride, offset);
}
