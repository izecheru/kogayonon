#include "layer/layer_stack.h"

namespace kogayonon
{
  LayerStack::~LayerStack()
  {
    m_layers.clear();
  }

  void LayerStack::pushLayer(std::shared_ptr<Layer> layer)
  {
    m_layers.push_back(std::move(layer));
  }

  void LayerStack::draw() const
  {
    for (auto& layer : m_layers)
    {
      if (!layer->isVisible())
      {
        // if the layer is disabled, dont render it
        continue;
      }
      else
      {
        layer->draw();
      }
    }
  }
} // namespace kogayonon