#pragma once
#include <vector>
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

    void serializeMeshes(const std::string& path);
    void deserializeMeshes(const std::string& path);

    std::vector<Mesh>& getMeshes();

    void operator=(const Model& other);
    inline bool isLoaded() { return m_loaded; }
    inline void setLoaded() { m_loaded = true; }
    inline std::string getPath()const { return m_path; }

  private:
    std::vector<Mesh> m_meshes;
    std::map<std::string, Texture> m_textures_loaded;
    std::string m_path;
    bool m_loaded = false;
  };
}