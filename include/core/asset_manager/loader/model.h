#pragma once
#include <map>
#include <vector>

#include "mesh.h"
#include "shader/shader.h"

namespace kogayonon
{
  class Model
  {
  public:
    explicit Model(const std::string& path_to_model);
    explicit Model(Model&& other) noexcept : m_loaded(other.m_loaded), m_path(other.m_path)
    {
      m_meshes = std::move(other.m_meshes);
    }

    Model() = default;

    // void draw(const Shader& shader);
    void init(const std::string& path) const;

    std::vector<Mesh>& getMeshes();

    Model(const Model&)            = default;
    Model& operator=(const Model&) = default;

    inline bool isLoaded() const
    {
      return m_loaded;
    }

    inline void setLoaded()
    {
      if (m_loaded == true)
        return;

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
} // namespace kogayonon