#pragma once

#include "layer.h"
#include "core/ui/imgui_interface.h"

namespace kogayonon
{
  class ImguiLayer :public Layer
  {
  public:
    ImguiLayer(GLFWwindow* window);

    bool initLayer(GLFWwindow* window);
    bool onEvent(Event& event) override;
    void onRender() override;
    void onUpdate() override;

  private:
    MyImguiInterface m_interface;
  };
}
