#pragma once

#include "core/ui/imgui_interface.h"
#include "event/keyboard_events.h"
#include "event/mouse_events.h"
#include "layer.h"

namespace kogayonon
{
  class ImguiLayer : public Layer
  {
  public:
    explicit ImguiLayer(GLFWwindow* window);

    bool initLayer(GLFWwindow* window);
    bool onKeyPressed(KeyPressedEvent& event);
    bool onMouseMoved(MouseMovedEvent& event);
    bool onMouseClicked(MouseClickedEvent& event);
    void draw() override;

  private:
    ImGuiInterface m_interface;
  };
} // namespace kogayonon
