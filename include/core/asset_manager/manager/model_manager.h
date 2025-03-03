#pragma once
#include <string>
#include <unordered_map>
#include <mutex>

#include "core/singleton/singleton.h"
#include "core/asset_manager/loader/model.h"

namespace kogayonon
{
  class ModelManager :public Singleton<ModelManager>
  {
  public:
    void addModel(const std::string& path);
    inline Model& getModel(const std::string& path) { return m_models[path]; }

  private:
    std::mutex m_mutex;
    std::unordered_map<std::string, Model> m_models;
  };
}
