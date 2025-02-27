#pragma once

#include <vector>
#include "core/asset_manager/model_loader/model.h"
#include "core/asset_manager/model_loader/model_manager.h"
#include "core/singleton/singleton.h"

namespace kogayonon
{
  class AssetManager :public Singleton<AssetManager>
  {
  public:
    void addModel(const std::string& path);
    void addTexture(const std::string& path);

    Model& getModel(const std::string& path);
  };
}
