#pragma once

#include "layer.h"
#include "core/ui/imgui_interface.h"
#include "events/mouse_events.h"
#include "events/keyboard_events.h"

namespace kogayonon
{
  class ImguiLayer :public Layer
  {
  public:
    ImguiLayer(GLFWwindow* window);

    bool initLayer(GLFWwindow* window);
    bool onKeyPressed(KeyPressedEvent& event);
    bool onMouseMoved(MouseMovedEvent& event);
    bool onMouseClicked(MouseClickedEvent& event);
    void render() override;
    void onUpdate() override;

  private:
    ImguiInterface m_interface;
  };
}
