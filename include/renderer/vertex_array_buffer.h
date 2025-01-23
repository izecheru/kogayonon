#pragma once
#include <vector>
#include <glad/glad.h>
#include "vertex_buffer.h"

namespace kogayonon
{
  class VertexArrayBuffer {
  public:
    VertexArrayBuffer();

    void bind() const;
    void unbind() const;
    void delete_buffer() const;

    void linkAttrib(VertexBuffer& vbo, GLuint layout, GLuint size, GLenum type, GLsizeiptr stride, void* offset);

  private:
    GLuint m_id;
  };
}
