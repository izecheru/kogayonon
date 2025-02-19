#pragma once

#include "core/model_loader/model.h"
#include "core/singleton/singleton.h"

namespace kogayonon
{
  class ModelManager : public Singleton<ModelManager> {
  public:
    bool pushModel(const std::string& path);
    Model& getModel(const std::string& path);
    void setupModels();

  private:
    std::atomic<bool> m_loaded = false;
    std::unordered_map<std::string, Model> m_models;
  };
}
