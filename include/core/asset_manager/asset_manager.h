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
    void addModel(const std::string& path);
    void addTexture(const std::string& path);

    Model& getModel(std::string& path);
  };
}
