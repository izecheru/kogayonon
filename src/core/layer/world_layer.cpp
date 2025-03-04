#include "core/layer/world_layer.h"
#include "core/asset_manager/asset_manager.h"

namespace kogayonon
{
  void WorldLayer::draw()
  {
    drawModels();
  }

  void WorldLayer::drawModels()const
  {
    auto& map = AssetManager::getInstance().getModelMap();
    for(auto& [path, model] : map)
    {
      model.draw(m_shader);
    }
  }
}