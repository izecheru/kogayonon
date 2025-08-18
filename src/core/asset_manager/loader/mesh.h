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

  struct MeshGPU
  {
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
  };

  struct MeshData
  {
    std::vector<Vertex> m_vertices{};
    std::vector<uint32_t> m_indices{};
    std::vector<std::string> m_textures{};

    explicit MeshData(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<std::string> textures)
        : m_vertices(std::move(vertices)), m_indices(std::move(indices)), m_textures(std::move(textures))
    {}

    explicit MeshData(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) : m_vertices(vertices), m_indices(indices)
    {}

    ~MeshData() {}
  };

  struct Texture
  {
    unsigned int id = 0;
    std::string type;
    std::string path;
    int width = 0;
    int height = 0;
    int num_components = 0;
    std::vector<unsigned char> data;
    bool gamma = true;

    Texture() = default;

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

    explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const std::vector<std::string>& textures)
        : m_data(std::make_shared<MeshData>(vertices, indices, textures))
    {}

    explicit Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
        : m_data(std::make_shared<MeshData>(vertices, indices))
    {}

    ~Mesh()
    {
      m_data.reset();
      m_data = nullptr;
    }

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
    std::shared_ptr<MeshData> m_data;
    std::shared_ptr<MeshGPU> m_gpu;

    bool m_init = false;
  };
} // namespace kogayonon