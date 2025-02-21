#include "core/model_loader/model_manager.h"
#include <future>
#include <filesystem>

namespace kogayonon
{
  static void createModel(std::unordered_map<std::string, Model>* models, const std::string& path, std::atomic<bool>* done)
  {
    Model model(path);
    (*models)[path] = model; // Dereference pointer to modify the map
    done->store(false);
  }

  bool ModelManager::pushModel(const std::string& path)
  {
    assert(std::filesystem::exists(path) != false);

    if (isLoaded(path) || m_loading_in_progress.load())
      return false;

    m_loading_in_progress.store(true);

    m_future = std::async(std::launch::async, [this, path]() {
      createModel(&m_models, path, &m_loading_in_progress);
      m_loading_in_progress.store(false);
      });

    return true;
  }


  bool ModelManager::isLoaded(const std::string& path)
  {
    return m_models.find(path) != m_models.end();
  }

  Model* ModelManager::getModel(const std::string& path)
  {
    if (isLoaded(path))
    {
      return &m_models[path];
    }
    return nullptr;
  }

  void ModelManager::drawModels(Shader& shader)
  {
    shader.bind();
    for (auto& model : m_models)
    {
      for (Mesh& mesh : model.second.getMeshes())
      {
        if (mesh.isInit())
        {
          mesh.draw(shader);
        }
        else
        {
          mesh.setupMesh();
        }
      }
    }
    shader.unbind();
  }

}