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

    // currently we cannot remove textures
    void addTexture(const std::string& model_path, const cgltf_data* data);

    inline Texture& getTexture(const std::string& path)
    {
      return m_loaded_textures[path];
    }

    inline std::unordered_map<std::string, Texture>& getTextures()
    {
      return m_loaded_textures;
    }

    inline std::mutex& getTextureMapMutex()
    {
      return m_mutex;
    }

  private:
    std::mutex m_mutex;
    std::unordered_map<std::string, Texture> m_loaded_textures;
  };
}
