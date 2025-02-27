#pragma once

#include "core/asset_manager/model_loader/model.h"
#include "core/singleton/singleton.h"
#include <future>

namespace kogayonon
{
  class ModelManager : public Singleton<ModelManager>
  {
  public:
    bool pushModel(const std::string& path, std::function<void(Model&)>callback);
    Model& getModel(const std::string& path);

  private:
    std::mutex m_mutex;
    std::unordered_map<std::string, Model> m_models{};
  };
}
