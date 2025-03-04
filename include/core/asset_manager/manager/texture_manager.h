#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <cgltf/cgltf.h>

#include "core/singleton/singleton.h"
#include "core/asset_manager/loader/mesh.h"

namespace kogayonon
{
  class TextureManager :public Singleton<TextureManager>
  {
  public:
    void addTexture(const std::string& path, const cgltf_data* data);

    inline Texture& getTexture(const std::string& path)
    {
      return m_loaded_textures[path];
    }

    inline std::unordered_map<std::string, Texture>& getTextures()
    {
      return m_loaded_textures;
    }

  private:
    std::mutex m_mutex;
    std::unordered_map<std::string, Texture> m_loaded_textures;
  };
}
