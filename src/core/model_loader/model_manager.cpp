#include "core/model_loader/model_manager.h"
#include "core/task/task_manager.h"
#include <future>
#include <filesystem>

namespace kogayonon
{
  bool ModelManager::pushModel(const std::string& path)
  {
    // make sure file exists otherwise bad luck
    assert(std::filesystem::exists(path) != false);

    if (isLoaded(path))
      return false;

    TaskManager::getInstance().pushTask([this, path]()
      {
        std::lock_guard<std::mutex> lock(m_model_manager_mutex);
        m_models[path] = Model(path);
      }
    );

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