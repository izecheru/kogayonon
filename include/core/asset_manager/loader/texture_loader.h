#pragma once
#include <unordered_map>
#include <string>

#include "core/singleton/singleton.h"
#include "mesh.h"
#include <cgltf/cgltf.h>

namespace kogayonon
{
  class TextureLoader :public Singleton<TextureLoader>
  {
  public:
    void loadTexture(cgltf_texture* texture, const std::string& type, std::vector<Texture>& textures, const std::string& model_dir);

  private:
    std::unordered_map<std::string, Texture> m_loaded_textures;
  };
}
