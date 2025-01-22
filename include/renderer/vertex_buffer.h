#pragma once
#include <glad/glad.h>
#include <glfw3.h>

namespace kogayonon
{
  /// <summary>
  /// Uploads vertices to the gpu
  /// </summary>
  class VertexBuffer {
  public:
    VertexBuffer(GLfloat* vertices, GLsizeiptr size);
    ~VertexBuffer();

    void bind() const;
    void unbind() const;
    void delete_buffer() const;

    /// <summary>
    /// Uploads data to the GPU
    /// </summary>
    /// <param name="size">Size of the data in bytes</param>
    /// <param name="data">Pointer to the data</param>
    /// <param name="usage">Usage hint (e.g., GL_STATIC_DRAW)</param>
    void upload(GLenum target, GLsizeiptr size, const void* data, GLenum usage) const;
    void upload(GLfloat* vertices, GLsizeiptr size) const;

    GLuint getId();

  private:
    GLuint id;
    bool bound = false;
  };
}
