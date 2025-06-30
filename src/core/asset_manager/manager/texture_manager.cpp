#include "core/asset_manager/manager/texture_manager.h"
#include "core/asset_manager/loader/texture_loader.h"
#include "core/time_tracker/time_tracker.h"

namespace kogayonon
{
  void TextureManager::addTexture(const std::string& model_path, const cgltf_data* data)
  {
    TimeTracker::getInstance()->startCount("texture");
    TextureLoader::getInstance()->pushTexture(
      model_path,
      [](const Texture& texture)// called after loading the texture
      {
        TimeTracker::getInstance()->stopCount("texture");
        KLogger::log(LogType::INFO, "Texture ", texture.path, " loaded in:", TimeTracker::getInstance()->getDuration("texture").count());
      },
      m_mutex,
      m_loaded_textures,
      data);
  }
}