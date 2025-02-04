#include "core/renderer/ebo.h"
#include <glad/glad.h>

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

const unsigned int ElementsBufferObject::getCount() const {
  return indices.size();
}
