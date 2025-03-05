#include "core/asset_manager/manager/model_manager.h"
#include "core/asset_manager/loader/model_loader.h"
#include "core/time_tracker/time_tracker.h"
#include "core/asset_manager/manager/texture_manager.h"

namespace kogayonon
{
  void ModelManager::addModel(const cgltf_data* data, const std::string& model_path)
  {
    TimeTracker::getInstance().startCount("model");
    ModelLoader::getInstance().pushModel(
      data,
      model_path,
      [](Model& model)// called after loading the model
      {
        model.setLoaded();
        TimeTracker::getInstance().stopCount("model");
        Logger::logInfo("Model loaded in:", TimeTracker::getInstance().getDuration("model").count());
      },
      m_mutex,
      m_models
    );
  }
}