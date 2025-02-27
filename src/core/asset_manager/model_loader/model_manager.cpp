#include "core/asset_manager/model_loader/model_manager.h"
#include "core/asset_manager/model_loader/model.h"
#include "core/task/task_manager.h"
#include "core/serialize/mesh_serializer.h"
#include "core/serialize/serializer.h"

#include <future>
#include <filesystem>

namespace kogayonon
{
  bool ModelManager::pushModel(const std::string& path, std::function<void(Model&)>callback)
  {
    // make sure model file exists otherwise bad luck
    assert(std::filesystem::exists(path) != false);

    //construct the directory and binary file here
    std::filesystem::path model_path(path);
    std::string model_name = model_path.stem().string();
    std::filesystem::path model_dir = model_path.parent_path() / model_name;
    if (!std::filesystem::exists(model_dir))
    {
      std::filesystem::create_directories(model_dir);
    }

    std::filesystem::path bin_path = model_dir / (model_name + ".bin");

    // model was already serialized
    if (std::filesystem::exists(bin_path))
    {
      TaskManager::getInstance().runTask([this, path, bin_path, callback]()
        {
          std::unique_lock<std::mutex> lock(m_mutex);
          Model model;
          model.deserializeMeshes(bin_path.string());
          m_models[path] = std::move(model);
          callback(m_models[path]);
        }
      );
    }
    else
    { // model needs to be serialized
      MeshSerializer::getInstance().openFile(bin_path.string(), FileMode::WRITE);
      MeshSerializer::getInstance().closeFile();
      TaskManager::getInstance().runTask([this, path, bin_path, callback]()
        {
          std::unique_lock<std::mutex> lock(m_mutex);
          Model model(path);
          model.serializeMeshes(bin_path.string());
          m_models[path] = std::move(model);
          callback(m_models[path]);
        }
      );
    }
    return true;
  }

  Model& ModelManager::getModel(const std::string& path)
  {
    return m_models[path];
  }
}