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
    glm::vec2 tex_coords;
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
    using texture_vec = std::vector<std::string>;
    using indices_vec = std::vector<uint32_t>;
    using vertice_vec = std::vector<Vertex>;

  public:
    Mesh() = default;
    explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<std::string>& textures);
    explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    void setupMesh();
    void draw(const Shader& shader);
    inline bool isInit() const
    {
      return m_init;
    }
    vertice_vec& getVertices();
    indices_vec& getIndices();
    texture_vec& getTextures();

  private:
    void setupTextures();

  private:
    vertice_vec m_vertices;
    texture_vec m_textures;
    indices_vec m_indices;

    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
    uint32_t m_ebo = 0;
    bool m_init = false;
  };
}