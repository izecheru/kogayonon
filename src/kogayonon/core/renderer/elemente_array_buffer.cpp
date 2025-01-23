#include "renderer/element_array_buffer.h"
#include "core/logger.h"

namespace kogayonon
{

  ElementArrayBuffer::ElementArrayBuffer(GLuint* indices, GLsizeiptr size) {
    m_size = size;
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
  }

  void ElementArrayBuffer::bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_id);
  }

  void ElementArrayBuffer::unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }

  void ElementArrayBuffer::delete_buffer() const {
    glDeleteBuffers(1, &m_id);
  }

  GLuint ElementArrayBuffer::getId()const {
    return m_id;
  }

  size_t ElementArrayBuffer::getSize() const {
    return m_size;
  }
}
