#pragma once
#include <memory>
#include "core/renderer/vao.h"
#include "core/renderer/vbo.h"
#include "core/renderer/ebo.h"
#include <glad/glad.h>
#include "texture.h"
#include "shader/shader.h"
#include "core/renderer/camera.h"
#include "core/logger.h"

class Mesh
{
private:
  struct MeshBuffers
  {
    std::unique_ptr<VertexArrayObject> vao;
    std::unique_ptr<VertexBufferObject> vbo;
    std::unique_ptr<ElementsBufferObject> ebo;

    void bindBuffers() {
      this->vao->bind();
      this->vbo->bind();
      this->ebo->bind();
    }

    void unbindBuffers() {
      this->vao->unbind();
      this->vbo->unbind();
      this->ebo->unbind();
    }


    MeshBuffers(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
      this->ebo = std::make_unique<ElementsBufferObject>(indices);
      this->vbo = std::make_unique<VertexBufferObject>(vertices);
      this->vao = std::make_unique<VertexArrayObject>();
    }
  };

public:
  Mesh() = default;
  Mesh(const std::vector<Vertex>& vertices, std::vector<unsigned int>& indices);

  void setupMesh();
  void draw();

private:
  std::vector<Vertex> m_vertices;
  std::vector<Texture> m_textures;
  std::vector<unsigned int> m_indices;
  MeshBuffers mesh_buffers;
};