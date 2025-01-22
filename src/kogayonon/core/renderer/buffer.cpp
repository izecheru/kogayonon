#include "renderer/vertex_buffer.h"

namespace kogayonon
{
  VertexBuffer::VertexBuffer(GLfloat* vertices, GLsizeiptr size) {
    glGenBuffers(1, &id);
    bind();
    upload(vertices, size);
  }

  VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &id);
  }

  void VertexBuffer::bind() const {
    // bind only if it is unbound
    if (!bound) {
      glBindBuffer(GL_ARRAY_BUFFER, id);
      bound != bound;
    }
  }

  void VertexBuffer::unbind() const {
    // unbind only if it is bound
    if (bound) {
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
  }

  void VertexBuffer::delete_buffer() const {
    glDeleteBuffers(1, &id);
  }

  void VertexBuffer::upload(GLenum target, GLsizeiptr size, const void* data, GLenum usage) const {
    bind();
    glBufferData(target, size, data, usage);
  }

  void VertexBuffer::upload(GLfloat* vertices, GLsizeiptr size) const {
    bind();
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
  }

  GLuint VertexBuffer::getId() {
    return id;
  }
}
