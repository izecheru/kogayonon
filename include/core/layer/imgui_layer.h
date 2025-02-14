#pragma once

#include "layer.h"
#include "core/ui/imgui_interface.h"
#include "events/keyboard_events.h"

namespace kogayonon
{
  class ImguiLayer :public Layer
  {
  public:
    ImguiLayer(GLFWwindow* window);

    bool initLayer(GLFWwindow* window);
    bool onKeyPressed(KeyPressedEvent& event);
    void onRender() override;
    void onUpdate() override;

  private:
    MyImguiInterface m_interface;
  };
}
