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
    unsigned int id = 0;
    std::string type;
    std::string path;
    int width, height, num_components;
    unsigned char* data;
    bool gamma = true;

    Texture() = default;
    ~Texture() = default;
    Texture(const std::string& t, const std::string& p, int w, int h, int n, unsigned char* d, bool g)
      :type(t), path(p), width(w), height(h), num_components(n), data(d), gamma(g)
    {
      if (data == nullptr) Logger::logError("data is null");
      Logger::logInfo("t-", type, " w-", width, " h-", height, " n-", num_components);
    }
  };

  class Mesh
  {
  public:
    Mesh() = default;
    explicit Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::vector<Texture>& textures);

    void setupMesh();
    void draw(Shader& shader);
    bool isInit() { return m_init; }
  private:
    void setupTextures();

  private:
    std::vector<Vertex> m_vertices;
    std::vector<Texture> m_textures;
    std::vector<unsigned int> m_indices;

  private:
    uint32_t m_vao;
    uint32_t m_vbo;
    uint32_t m_ebo;
    uint32_t m_num_indices = 0;
    bool m_init = false;
  };
}