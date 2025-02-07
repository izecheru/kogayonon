#pragma once
#include <vector>
#include <assimp/scene.h>
#include "shader/shader.h"
#include "mesh.h"

namespace kogayonon
{
  class Model
  {
  public:
    Model(std::string& path_to_model, Shader& shader);
    ~Model() = default;

    void render(Shader& shader);
    void init(std::string& path, Shader& shader);

  private:
    std::vector<Mesh> m_meshes;
    std::vector<Texture> m_textures_loaded;
    std::string path;
  };
}