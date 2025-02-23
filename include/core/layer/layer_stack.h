#pragma once

#include <vector>
#include <memory>
#include "layer.h"

namespace kogayonon
{
  class LayerStack
  {
  public:
    ~LayerStack();
    void pushLayer(std::unique_ptr<Layer> layer);
    void draw() const;

  private:
    std::vector<std::unique_ptr<Layer>> m_layers{};
  };
}
