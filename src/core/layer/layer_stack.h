#pragma once
#include <memory>
#include <vector>

#include "layer.h"

namespace kogayonon
{
  class LayerStack
  {
  public:
    LayerStack() = default;
    ~LayerStack();

    void pushLayer(std::unique_ptr<Layer> layer);
    void draw() const;

  private:
    std::vector<std::unique_ptr<Layer>> m_layers{};
  };
} // namespace kogayonon
