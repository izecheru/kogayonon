#include "core/renderer/ebo.h"
#include <glad/glad.h>

ElementsBufferObject::ElementsBufferObject(ElementsBufferObject&& other) noexcept {
  m_id = other.m_id;
  other.m_id = 0;
}

ElementsBufferObject& ElementsBufferObject::operator=(ElementsBufferObject&& other) noexcept {
  if (this != &other)
  {
    glDeleteBuffers(1, &m_id);
    m_id = other.m_id;
    other.m_id = 0;
  }
  return *this;
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
