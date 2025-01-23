#include <vector>
#include "core/logger.h"
#include "renderer/vertex_buffer.h"

namespace kogayonon
{
  VertexBuffer::VertexBuffer(GLfloat* vertices, GLsizeiptr size) {
    m_size = size;
    Logger::logInfo("[vertex buffer] size = ", size);
    glGenBuffers(1, &m_id);
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
  }

  void VertexBuffer::bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_id);
  }

  void VertexBuffer::unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  void VertexBuffer::delete_buffer() const {
    glDeleteBuffers(1, &m_id);
  }

  GLuint VertexBuffer::getId() const {
    return m_id;
  }

  GLsizeiptr VertexBuffer::getSize() const {
    return m_size;
  }
}
