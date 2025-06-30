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
    Model(Model&& other)noexcept;
    Model() = default;
    ~Model() = default;

    void draw(const Shader& shader);
    void init(const std::string& path)const;

    std::vector<Mesh>& getMeshes();
    void operator=(const Model& other);

    inline bool isLoaded()const
    {
      return m_loaded;
    }

    inline void setLoaded()
    {
      m_loaded = true;
    }

    inline std::string& getPath()
    {
      return m_path;
    }

  private:
    std::vector<Mesh> m_meshes;
    std::string m_path;
    bool m_loaded = false;
  };
}