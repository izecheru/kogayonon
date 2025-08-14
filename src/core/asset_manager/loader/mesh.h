#pragma once
#include <vector>

#include "shader/shader.h"

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
    std::string type; // Changed from char array to std::string
    std::string path; // Changed from char array to std::string
    int width          = 0;
    int height         = 0;
    int num_components = 0;
    std::vector<unsigned char> data; // Changed from raw pointer to vector for automatic memory management
    bool gamma = true;

    // Default constructor
    Texture() = default;

    // Parameterized constructor
    explicit Texture(const std::string& t, const std::string& p, int w, int h, int n, const std::vector<unsigned char>& d, bool g)
        : type(t), path(p), width(w), height(h), num_components(n), data(d), gamma(g)
    {}

    inline std::string getPath() const
    {
      return path;
    }
  };

  class Mesh
  {
  public:
    Mesh() = default;
    explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<std::string>& textures);
    explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

    void setupMesh();
    inline bool isInit() const
    {
      return m_init;
    }

    std::vector<Vertex>& getVertices();
    std::vector<uint32_t>& getIndices();
    std::vector<std::string>& getTextures();

  private:
    bool setupTextures();

  private:
    std::vector<Vertex> m_vertices{};
    std::vector<uint32_t> m_indices{};
    std::vector<std::string> m_textures{};

    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
    uint32_t m_ebo = 0;
    bool m_init    = false;
  };
} // namespace kogayonon