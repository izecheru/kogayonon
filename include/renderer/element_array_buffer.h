#pragma once
#include <glad/glad.h>

namespace kogayonon
{
  class ElementArrayBuffer {
  public:
    ElementArrayBuffer(GLuint* indices, GLsizeiptr size);

    void bind()const;
    void unbind()const;
    void delete_buffer() const;

    GLuint getId() const;
    size_t getSize() const;

  private:
    GLuint m_id;
    size_t m_size;
  };
}