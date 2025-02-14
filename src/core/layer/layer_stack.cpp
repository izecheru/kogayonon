#include "core/layer/layer_stack.h"
#include "core/layer/imgui_layer.h"

using namespace kogayonon;

void kogayonon::LayerStack::pushLayer(std::unique_ptr<Layer> layer) {
  m_layers.push_back(std::move(layer));
}

bool LayerStack::handleEvent(Event& event) {
  for (auto& it = m_layers.begin(); it != m_layers.end(); it++)
  {
    if ((*it->get()).onEvent(event))
    {
      // it got handled
      return true;
    }
  }
  return false;
}

//TODO implement layer rendering somehow
void LayerStack::render() {
  for (auto& it = m_layers.begin(); it != m_layers.end(); it++)
  {
    it->get()->onRender();
  }
}

