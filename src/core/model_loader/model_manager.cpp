#include "core/model_loader/model_manager.h"

namespace kogayonon
{
  bool ModelManager::pushModel(const std::string& path) {
    if (m_models.find(path) != m_models.end()) {
      return false;
    }

    if (m_loaded.load()) {
      return false;
    }

    m_loaded.store(true);

    std::thread loader_thread([this, path]()
      {
        try {
          Model model(path);
          m_models[path] = model;
        }
        catch (const std::exception& err) {
          Logger::logError(err.what());
        }
      });

    loader_thread.detach();

    return false;
  }

  Model& ModelManager::getModel(const std::string& path) {
    return m_models[path];
  }

  void ModelManager::setupModels() {
    for (auto& model : m_models) {
      for (auto& mesh : model.second.getMeshes()) {
        mesh.setupMesh();
      }
    }
  }
}