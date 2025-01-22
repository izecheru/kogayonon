#pragma once
#include <vector>
#include <glad/glad.h>
#include "vertex_buffer.h"

namespace kogayonon
{
  class VertexArrayBuffer {
  public:
    VertexArrayBuffer();
    ~VertexArrayBuffer();

    void bind() const;
    void unbind() const;

    /// <summary>
    /// Links a VBO and defines its layout
    /// </summary>
    /// <param name="vbo">The vertex buffer</param>
    /// <param name="attribute_index">Attribute index in the shader</param>
    /// <param name="size">Number of components per vertex attribute</param>
    /// <param name="type">Data type of components (e.g., GL_FLOAT)</param>
    /// <param name="normalized">Should fixed-point data be normalized</param>
    /// <param name="stride">Byte offset between consecutive attributes</param>
    /// <param name="offset">Offset of the first attribute in the buffer</param>
    void addVertexBuffer(const VertexBuffer& vbo, GLuint attribute_index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* offset);
    void linkAttrib(VertexBuffer& vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);

  private:
    GLuint id;
    std::vector<const VertexBuffer*> buffers;
  };
}
