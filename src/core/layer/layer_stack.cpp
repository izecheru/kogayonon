#include "core/layer/layer_stack.h"
#include "core/layer/imgui_layer.h"

namespace kogayonon
{
  LayerStack::~LayerStack()
  {

  }

  void LayerStack::pushLayer(std::unique_ptr<Layer> layer)
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
}