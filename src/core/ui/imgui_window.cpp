#include "core/ui/imgui_window.h"
#include <imgui-1.91.8/imgui.h>
#include "core/renderer/camera.h"

namespace kogayonon
{
  void ImguiWindow::setDocked(bool status) {
    m_props.m_docked = status;
  }

  void ImguiWindow::setVisible(bool status) {
    m_props.m_visible = status;
  }

  void ImguiWindow::setX(double x) {
    m_props.m_x = x;
  }

  void ImguiWindow::setY(double y) {
    m_props.m_y = y;
  }

  void ImguiWindow::draw() {
    ImGui::Begin(m_props.m_name.c_str(), nullptr, m_props.m_flags);
    ImGui::Text("Some settings");
    ImGui::SliderFloat("Mouse sensitivity", &Camera::getInstance().getProps().mouse_sens, 0.01f, 10.0f, "%.4f");
    ImGui::SliderFloat("Movement sensitivity", &Camera::getInstance().getProps().movement_speed, 0.01f, 10.0f, "%.4f");
    ImGui::Checkbox("Docked?", &m_props.m_docked);
    if (ImGui::Checkbox("Can move?", &m_props.m_can_move)) {
      if (!m_props.m_can_move) {
        m_props.m_flags |= ImGuiWindowFlags_NoMove;
      }
      else {
        m_props.m_flags &= ~ImGuiWindowFlags_NoMove;
      }
    }
    ImGui::End();
  }
}