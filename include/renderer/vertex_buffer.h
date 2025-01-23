#pragma once
#include <glad/glad.h>
#include <glfw3.h>
#include <vector>

namespace kogayonon
{
  class VertexBuffer {
  public:
    VertexBuffer(GLfloat* vertices, GLsizeiptr size);

    void bind() const;
    void unbind() const;
    void delete_buffer() const;

    GLuint getId() const;
    GLsizeiptr getSize() const;
  private:
    GLuint m_id;
    GLsizeiptr m_size;
  };
}
