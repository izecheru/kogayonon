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

    // Method to check if a model is loaded
    bool isLoaded(const std::string& path);

    // Retrieve the model once it's loaded
    Model* getModel(const std::string& path);
    void drawModels(Shader& shader);
    bool stillLoading() { return m_loading_in_progress; }

    void initModel();

  private:
    std::atomic<bool> m_loading_in_progress{ false };
    std::unordered_map<std::string, Model> m_models;
    std::future<void> m_future;
  };
}
