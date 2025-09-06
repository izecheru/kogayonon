#pragma once
#include <map>
#include <vector>

#include "resources/mesh.h"

namespace kogayonon_resources {
class Model
{
  public:
    explicit Model( const std::string& path_to_model );
    explicit Model( Model&& other ) noexcept;
    Model() = default;

    Model( const Model& ) = default;
    Model& operator=( const Model& ) = default;

    void init( const std::string& path ) const;

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

    inline std::string& getPath()
    {
        return m_path;
    }

  private:
    std::vector<Mesh> m_meshes;
    std::string m_path;
    bool m_loaded = false;
};
} // namespace kogayonon_resources