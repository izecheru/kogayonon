#pragma once
#include "core/renderer/vao.h"
#include "core/renderer/vbo.h"
#include "core/renderer/ebo.h"
#include <glad/glad.h>
#include "texture.h"
#include "shader/shader.h"
#include "core/renderer/camera.h"

class Mesh
{
public:
  Mesh() {}
  Mesh(const std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures);

  void setupMesh();
  void draw();

private:
  VertexArrayObject m_vao;
  std::vector<Vertex> vertices;
  std::vector<Texture> textures;
  std::vector<unsigned int> indices;
};