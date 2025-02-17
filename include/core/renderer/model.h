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
    Model(std::string& path_to_model, Shader& shader);
    ~Model() = default;

    void draw(Shader& shader);
    void init(std::string& path, Shader& shader);

  private:
    std::vector<Mesh> m_meshes;
    std::map<std::string, Texture> m_textures_loaded;
    std::string path;
  };
}