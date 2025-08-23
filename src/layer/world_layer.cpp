#include "layer/world_layer.h"

#include "asset_manager/asset_manager.h"

namespace kogayonon
{
  void WorldLayer::draw()
  {
    drawModels();
  }

  void WorldLayer::drawModels() const
  {
    // auto& map = ;
    // for (auto& [path, model] : map)
    //{
    //   for (Mesh& mesh : model.getMeshes())
    //   {
    //     mesh.draw(m_shader);
    //   }
    // }
  }
} // namespace kogayonon