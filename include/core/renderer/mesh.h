#pragma once
#include <core/renderer/buffer.h>
#include <glad/glad.h>

class Mesh
{
public:
  Mesh() {}
  Mesh(const std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) :m_vbo(vertices), m_vao(), m_ebo(indices) {
    // here the vao is set up and we just bin and draw and unbind after
    m_vao.bind();
    m_vbo.bind();

    // TODO must add textures as well
    m_vao.attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // Vertex positions
    m_vao.attribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, colors)); // Vertex colors 

    m_ebo.bind();

    m_vao.unbind();
    m_vbo.unbind();
    m_ebo.unbind();
  }

  void draw();

private:
  VertexBufferObject m_vbo;
  VertexArrayObject m_vao;
  ElementsBufferObject m_ebo;
};