#pragma once
#include <unordered_map>
#include <string>
#include <cgltf/cgltf.h>
#include <mutex>

#include "core/singleton/singleton.h"
#include "mesh.h"

namespace kogayonon
{
  class TextureLoader :public Singleton<TextureLoader>
  {
  public:
    void loadTexture(const cgltf_texture* texture, std::unordered_map<std::string, Texture>& loaded_textures, const std::string& type, const std::string& model_dir);
    void pushTexture(const std::string& texture_path, std::function<void(const Texture&)> callback, std::mutex& mutex, std::unordered_map<std::string, Texture>& textures, const cgltf_data* data);
    void initializeTextures(const cgltf_data* data, const std::string& model_dir, std::unordered_map<std::string, Texture>& loaded_textures);
    void processMaterial(const cgltf_material* material, std::unordered_map<std::string, Texture>& loaded_textures, const std::string& model_dir);
  };
}
