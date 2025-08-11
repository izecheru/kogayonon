#pragma once

#include <cgltf.h>

#include <mutex>
#include <vector>

#include "core/asset_manager/loader/model.h"
#include "core/klogger/klogger.h"

namespace kogayonon
{
  class AssetManager
  {
  public:
    AssetManager()  = default;
    ~AssetManager() = default;

    void initializeModel(const std::string& path);

    // Functions needed for already serialized models
    void addModel(const std::string& path);
    void addTexture(const std::string& path);

    // Functions from previous managers since i overabstracted things
    void addModel(const cgltf_data* data, const std::string& model_path);
    void addTexture(const std::string& model_path, const cgltf_data* data);

  public:
    Model& getModel(const std::string& path);
    std::unordered_map<std::string, Model>& getModelMap();

  private:
    cgltf_data* m_data = nullptr;
    std::mutex m_model_map_mutex;
    std::unordered_map<std::string, Model> m_models;

    std::mutex m_texture_map_mutex;
    std::unordered_map<std::string, Texture> m_loaded_textures;
  };
} // namespace kogayonon
