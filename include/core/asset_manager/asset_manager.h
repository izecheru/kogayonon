#pragma once

#include <vector>
#include "core/asset_manager/loader/model.h"
#include "core/singleton/singleton.h"
#include <mutex>
#include <cgltf/cgltf.h>

namespace kogayonon
{
  class AssetManager :public Singleton<AssetManager>
  {
  public:
    void addModel(const std::string& path)const;
    void addTexture(const std::string& path)const;
    void initializeModel(const std::string& path);

    Model& getModel(const std::string& path)const;
    std::unordered_map<std::string, Model>& getModelMap()const;
  private:
    cgltf_data* m_data = nullptr;
  };
}
