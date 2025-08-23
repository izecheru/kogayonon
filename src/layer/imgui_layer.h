#pragma once

#include "input/keyboard_events.h"
#include "input/mouse_events.h"
#include "layer.h"
#include "ui/imgui_window.h"

namespace kogayonon
{
  class ImguiLayer : public Layer
  {
  public:
    ImguiLayer();

    void draw() override;

    void push_window(std::shared_ptr<ImGuiWindow>& window);
    std::vector<std::shared_ptr<ImGuiWindow>>& getWindows();

  private:
    std::vector<std::shared_ptr<ImGuiWindow>> m_windows{};
  };
} // namespace kogayonon
