#include "core/ui/win_camera_settings.h"
#include "core/renderer/camera.h"

namespace kogayonon
{
  bool CameraSettingsWindow::initCameraWindow() {
    return false;
  }

  void CameraSettingsWindow::draw() {
    ImGui::Begin(m_props->m_name.c_str(), nullptr, m_props->m_flags);
    ImGui::Text("Camera settings");
    ImGui::SliderFloat("Mouse sensitivity", &Camera::getInstance().getProps().mouse_sens, 0.01f, 10.0f, "%.4f");
    ImGui::SliderFloat("Movement sensitivity", &Camera::getInstance().getProps().movement_speed, 0.01f, 10.0f, "%.4f");
    if (ImGui::Checkbox("Can move the window?", &m_props->m_can_move)) {
      if (!m_props->m_can_move) {
        m_props->m_flags |= ImGuiWindowFlags_NoMove;
      }
      else {
        m_props->m_flags &= ~ImGuiWindowFlags_NoMove;
      }
    }
    ImGui::End();
  }
}