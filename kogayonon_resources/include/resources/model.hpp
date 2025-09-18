#pragma once
#include <vector>

#include "resources/mesh.hpp"

namespace kogayonon_resources
{
class Model
{
public:
  explicit Model( std::vector<Mesh>& meshes );
  explicit Model( Model&& other ) noexcept;
  Model() = default;

  std::vector<Mesh>& getMeshes();

  inline bool isLoaded() const
  {
    return m_loaded;
  }

  inline void setLoaded()
  {
    if ( m_loaded == true )
      return;

    m_loaded = true;
  }

private:
  std::vector<Mesh> m_meshes;
  bool m_loaded = false;
};
} // namespace kogayonon_resources