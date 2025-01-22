#include "renderer/vertex_array_buffer.h"
#include "renderer/vertex_buffer.h"
#include "core/logger.h"
#include <glad/glad.h>

namespace kogayonon
{

  VertexArrayBuffer::~VertexArrayBuffer() {
    glDeleteVertexArrays(1, &id);
  }

  VertexArrayBuffer::VertexArrayBuffer() {
    glGenVertexArrays(1, &id);
  }

  void VertexArrayBuffer::bind() const {
    glBindVertexArray(id);
  }

  void VertexArrayBuffer::unbind() const {
    glBindVertexArray(0);
  }

  void VertexArrayBuffer::addVertexBuffer(const VertexBuffer& vbo, GLuint attribute_index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset) {
    bind();
    vbo.bind();
    glVertexAttribPointer(attribute_index, size, type, normalized, stride, offset);
    glEnableVertexAttribArray(attribute_index);
    buffers.push_back(&vbo);
    vbo.unbind();
    unbind();
  }

  void VertexArrayBuffer::linkAttrib(VertexBuffer& vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset) {
    bind();
    vbo.bind();
    glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(layout);
    vbo.unbind();
    unbind();
  }

}
