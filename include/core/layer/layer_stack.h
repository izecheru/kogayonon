#pragma once

#include <vector>
#include <memory>
#include "layer.h"

namespace kogayonon
{
  class LayerStack
  {
  public:
    void pushLayer(Layer* layer);
    void render() const;

  private:
    std::vector<Layer*> m_layers{};
  };
}
