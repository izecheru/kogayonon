#pragma once
#include "core/renderer/vao.h"
#include "core/renderer/vbo.h"
#include "core/renderer/ebo.h"
#include <glad/glad.h>
#include "texture.h"

class Mesh
{
public:
  Mesh() {}
  Mesh(const char* texture_path, const std::vector<Vertex>& vertices, std::vector<unsigned int>& indices) :m_vbo(vertices), m_vao(), m_ebo(indices), m_texture(texture_path) {
    // here the vao is set up and we just bin and draw and unbind after
    m_vao.bind();
    m_vbo.bind();
    m_ebo.bind();
    m_texture.bind();

    m_vao.attribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); // Vertex positions
    m_vao.attribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, colors)); // Vertex colors 
    m_vao.attribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture)); // Vertex colors 


    m_vao.unbind();
    m_vbo.unbind();
    m_ebo.unbind();
    m_texture.unbind();
  }

  void draw();

private:
  VertexBufferObject m_vbo;
  VertexArrayObject m_vao;
  ElementsBufferObject m_ebo;
  Texture m_texture;
};