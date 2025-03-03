
#include "core/asset_manager/manager/model_manager.h"
#include "core/asset_manager/loader/model_loader.h"
#include "core/time_tracker/time_tracker.h"

namespace kogayonon
{
  void ModelManager::addModel(const std::string& path)
  {
    Timer::getInstance().startCount("model");
    ModelLoader::getInstance().pushModel(
      path,
      [](Model& model)// called after loading the model
      {
        model.setLoaded();
        Timer::getInstance().stopCount("model");
        Logger::logInfo("Model loaded in:", Timer::getInstance().getDuration("model").count());
      },
      m_mutex,
      m_models
    );
  }
}