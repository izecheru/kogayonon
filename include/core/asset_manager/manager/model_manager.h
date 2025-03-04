#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <cgltf/cgltf.h>

#include "core/singleton/singleton.h"
#include "core/asset_manager/loader/model.h"

namespace kogayonon
{
  class ModelManager :public Singleton<ModelManager>
  {
  public:
    void addModel(const cgltf_data* data, const std::string& model_path);

    inline std::unordered_map<std::string, Model>& getModelMap()
    {
      return m_models;
    }

    inline Model& getModel(const std::string& path)
    {
      return m_models[path];
    }

  private:
    std::mutex m_mutex;
    std::unordered_map<std::string, Model> m_models;
  };
}
