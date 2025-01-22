#include "renderer/element_array_buffer.h"

namespace kogayonon
{

  ElementArrayBuffer::ElementArrayBuffer(GLuint* indices, GLsizeiptr size) {
    glGenBuffers(1, &id);
    bind();
    upload(indices, size);
  }

  ElementArrayBuffer::~ElementArrayBuffer() {
    glDeleteBuffers(1, &id);
  }

  void ElementArrayBuffer::bind() const {
    // bind only if it is unbound
    if (!bound) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
      bound != bound;
    }
  }

  void ElementArrayBuffer::unbind() const {
    // unbind only if it is bound
    if (bound) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
  }

  void ElementArrayBuffer::delete_buffer() const {
    glDeleteBuffers(1, &id);
  }

  void kogayonon::ElementArrayBuffer::upload(GLuint* indices, GLsizeiptr size) const {
    bind();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
  }

  GLuint ElementArrayBuffer::getId() {
    return id;
  }
}
