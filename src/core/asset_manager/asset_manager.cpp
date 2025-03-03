#include "core/asset_manager/asset_manager.h"
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <filesystem>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "core/task/task_manager.h"
#include "core/serialize/mesh_serializer.h"
#include "core/time_tracker/time_tracker.h"
#include "core/asset_manager/loader/model_loader.h"
#include "core/asset_manager/manager/model_manager.h"

namespace kogayonon
{

  void AssetManager::addModel(const std::string& path)
  {
    ModelManager::getInstance().addModel(path);
  }

  void  AssetManager::addTexture(const std::string& path)
  {

  }

  Model& AssetManager::getModel(std::string& path)
  {
    return ModelManager::getInstance().getModel(path);
  }
}