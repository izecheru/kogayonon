#pragma once

#include <vector>
#include <memory>
#include "layer.h"

namespace kogayonon
{
  class LayerStack
  {
  public:
    void pushLayer(std::unique_ptr<Layer> layer);

    void popLayer(const std::unique_ptr<Layer>& layer);

    bool handleEvent(Event& event);

    void render();

  private:
    std::vector<std::unique_ptr<Layer>> m_layers;
  };
}
