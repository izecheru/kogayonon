#pragma once

#include "core/model_loader/model.h"
#include "core/singleton/singleton.h"
#include <future>

namespace kogayonon
{
  class ModelManager : public Singleton<ModelManager>
  {
  public:
    // Method to load a model in a separate thread
    bool pushModel(const std::string& path);

    bool pushSerializedModel(const std::string& path);

    // Method to check if a model is loaded
    bool isLoaded(const std::string& path);

    Model* getModel(const std::string& path);
    void drawModels(Shader& shader);

  private:
    std::mutex m_mutex;
    std::unordered_map<std::string, Model> m_models;
  };
}
