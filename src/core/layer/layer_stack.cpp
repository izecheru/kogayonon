#include "core/layer/layer_stack.h"
#include "core/layer/imgui_layer.h"

using namespace kogayonon;

void LayerStack::pushLayer(std::unique_ptr<Layer> layer) {
  m_layers.push_back(std::move(layer));
}

//bool LayerStack::handleEvent(Event& event) const {
//  for (auto& it = m_layers.begin(); it != m_layers.end(); it++)
//  {
//    if ((*it->get()).onKeyPressed(KeyPressedEvent))
//    {
//      // it got handled
//      return true;
//    }
//  }
//  return false;
//}

void LayerStack::render() const {
  for (auto& it = m_layers.begin(); it != m_layers.end(); it++)
  {
    if (!(*it->get()).isVisible())
    {
      // if the layer is disabled dont render it
      continue;
    }
    else
    {
      it->get()->onRender();
    }
  }
}