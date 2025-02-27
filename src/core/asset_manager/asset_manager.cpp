#include "core/asset_manager/asset_manager.h"

namespace kogayonon
{
  void AssetManager::addModel(const std::string& path)
  {
    ModelManager& manager = ModelManager::getInstance();
    manager.pushModel(path, [](Model& model) { model.setLoaded(); });
  }

  void  AssetManager::addTexture(const std::string& path)
  {

  }

  Model& AssetManager::getModel(const std::string& path)
  {
    return ModelManager::getInstance().getModel(path);
  }
}