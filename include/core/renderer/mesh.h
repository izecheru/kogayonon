#pragma once
#include <glad/glad.h>
#include <vector>
#include "shader/shader.h"
#include "core/renderer/camera.h"
#include "core/logger.h"

#include <assimp\types.h>
namespace kogayonon
{
  struct Vertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture;
    glm::vec3 tangent;
    glm::vec3 bitangent;
  };

  struct Texture
  {
    unsigned int id;
    std::string type;
    std::string path;
  };

  class Mesh
  {
  public:
    Mesh() = default;
    Mesh(const std::vector<Vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures);

    void setupMesh();
    void render(Shader& shader);
  private:
    std::vector<Vertex> m_vertices;
    std::vector<Texture> m_textures;
    std::vector<unsigned int> m_indices;

  private:
    unsigned int m_vao, m_vbo, m_ebo;
  };
}