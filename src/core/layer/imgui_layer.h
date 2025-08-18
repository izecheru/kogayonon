#pragma once

#include "core/ui/imgui_window.h"
#include "event/keyboard_events.h"
#include "event/mouse_events.h"
#include "layer.h"

namespace kogayonon
{
  class ImguiLayer : public Layer
  {
  public:
    ImguiLayer();

    bool onKeyPressed(KeyPressedEvent& event);
    bool onMouseMoved(MouseMovedEvent& event);
    bool onMouseClicked(MouseClickedEvent& event);
    void draw() override;

    void push_window(std::shared_ptr<ImGuiWindow>& window);
    std::vector<std::shared_ptr<ImGuiWindow>>& getWindows();

  private:
    std::vector<std::shared_ptr<ImGuiWindow>> m_windows{};
  };
} // namespace kogayonon
