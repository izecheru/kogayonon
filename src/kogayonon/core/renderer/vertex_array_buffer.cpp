#include "renderer/vertex_array_buffer.h"
#include "renderer/vertex_buffer.h"
#include "core/logger.h"
#include <glad/glad.h>

namespace kogayonon
{

  VertexArrayBuffer::VertexArrayBuffer() {
    glGenVertexArrays(1, &m_id);
  }

  void VertexArrayBuffer::bind() const {
    glBindVertexArray(m_id);
  }

  void VertexArrayBuffer::unbind() const {
    glBindVertexArray(0);
  }

  void VertexArrayBuffer::delete_buffer() const {
    glDeleteVertexArrays(1, &m_id);
  }

  void VertexArrayBuffer::linkAttrib(VertexBuffer& vbo, GLuint layout, GLuint size, GLenum type, GLsizeiptr stride, void* offset) {
    bind();
    vbo.bind();
    glVertexAttribPointer(layout, size, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    vbo.unbind();
    unbind();
    Logger::logInfo("Enabled attribute = ", layout);
  }
}
