#pragma once
#include <glad/glad.h>
#include <vector>
#include "shader/shader.h"
#include "core/renderer/camera.h"
#include "core/logger.h"


namespace kogayonon
{
  struct Vertex
  {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texture;
  };
  static_assert(sizeof(Vertex) == (sizeof(glm::vec3) * 2 + sizeof(glm::vec2)));

  struct Texture
  {
    unsigned int id = 0;
    char type[32];
    char path[256];
    int width, height, num_components;
    unsigned char* data;
    bool gamma = true;

    Texture() = default;
    ~Texture() = default;
    Texture(const std::string& t, const std::string& p, int w, int h, int n, unsigned char* d, bool g)
      : width(w), height(h), num_components(n), data(d), gamma(g)
    {
      strncpy_s(type, t.c_str(), sizeof(type) - 1);
      type[sizeof(type) - 1] = '\0';
      strncpy_s(path, p.c_str(), sizeof(path) - 1);
      path[sizeof(path) - 1] = '\0';
    }
  };

  class Mesh
  {
  private:
    using texture_vec = std::vector<Texture>;
    using indices_vec = std::vector<uint32_t>;
    using vertice_vec = std::vector<Vertex>;
  public:
    Mesh() = default;
    explicit Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture> textures);

    void setupMesh();
    void draw(Shader& shader);
    bool isInit() { return m_init; }
    vertice_vec& getVertices();
    indices_vec& getIndices();
    texture_vec& getTextures();

  private:
    void setupTextures();

  private:
    vertice_vec m_vertices;
    texture_vec m_textures;
    indices_vec m_indices;

  private:
    uint32_t m_vao;
    uint32_t m_vbo;
    uint32_t m_ebo;
    uint32_t m_num_indices = 0;
    bool m_init = false;
  };
}