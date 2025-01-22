#pragma once
#include <glad/glad.h>

namespace kogayonon
{
  class ElementArrayBuffer {

  public:
    ElementArrayBuffer(GLuint* vertices, GLsizeiptr size);
    ~ElementArrayBuffer();

    void bind() const;
    void unbind() const;
    void delete_buffer() const;

    void upload(GLuint* indices, GLsizeiptr size) const;

    GLuint getId();

  private:
    GLuint id;
    bool bound = false;
  };
}