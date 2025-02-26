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

  struct Texture
  {
    unsigned int id = 0;
    std::string type;  // Changed from char array to std::string
    std::string path;  // Changed from char array to std::string
    int width = 0;
    int height = 0;
    int num_components = 0;
    std::vector<unsigned char> data;  // Changed from raw pointer to vector for automatic memory management
    bool gamma = true;

    // Default constructor
    Texture() = default;

    // Parameterized constructor
    Texture(const std::string& t, const std::string& p, int w, int h, int n, const std::vector<unsigned char>& d, bool g)
      : type(t), path(p), width(w), height(h), num_components(n), data(d), gamma(g)
    {
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
    inline bool isInit() { return m_init; }
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