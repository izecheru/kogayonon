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
#include "core/asset_manager/manager/texture_manager.h"

namespace kogayonon
{
  void AssetManager::addModel(const std::string& path) const
  {
    ModelManager::getInstance().addModel(m_data, path);
  }

  void AssetManager::addTexture(const std::string& path)const
  {
    TextureManager::getInstance().addTexture(path, m_data);
  }

  void AssetManager::initializeModel(const std::string& path)
  {
    std::string model_dir = std::filesystem::path(path).parent_path().string();
    cgltf_options options = {};
    if(cgltf_result result = cgltf_parse_file(&options, path.c_str(), &m_data); result != cgltf_result_success)
    {
      KLogger::log(LogType::ERROR, "Failed to load glTF file:", path);
    }

    cgltf_load_buffers(&options, m_data, path.c_str());
    assert(m_data != nullptr);

    addModel(path);
  }

  Model& AssetManager::getModel(const std::string& path) const
  {
    return ModelManager::getInstance().getModel(path);
  }
  std::unordered_map<std::string, Model>& AssetManager::getModelMap() const
  {
    return ModelManager::getInstance().getModelMap();
  }
}