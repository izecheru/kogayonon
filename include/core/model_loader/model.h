#pragma once
#include <vector>
#include <assimp/scene.h>
#include "shader/shader.h"
#include "mesh.h"
#include <map>

namespace kogayonon
{
  class Model
  {
  public:
    explicit Model(const std::string& path_to_model);
    Model() = default;
    ~Model() = default;

    void draw(Shader& shader);
    void init(const std::string path);

    std::vector<Mesh>& getMeshes();

    void operator=(const Model& other);
    bool isLoaded() { return m_loaded; }

  private:
    std::vector<Mesh> m_meshes;
    std::map<std::string, Texture> m_textures_loaded;
    std::string path;
    bool m_loaded = false;
  };
}