#include "core/ui/win_camera_settings.h"
#include "core/renderer/camera.h"

namespace kogayonon
{
  bool CameraSettingsWindow::initCameraWindow() {
    return false;
  }

  void CameraSettingsWindow::draw() {
    Camera& m_camera = Camera::getInstance();
    ImGui::Begin(m_props->m_name.c_str(), nullptr, m_props->m_flags);
    ImGui::Text("Camera settings");

    drawCoordinates(m_camera.getX(), m_camera.getY(), m_camera.getZ());

    ImGui::SliderFloat("Mouse sensitivity", &m_camera.getProps().mouse_sens, 0.01f, 10.0f, "%.4f");
    ImGui::SliderFloat("Movement sensitivity", &m_camera.getProps().movement_speed, 0.01f, 10.0f, "%.4f");
    if (ImGui::Checkbox("Can move the window?", &m_props->can_move)) {
      if (!m_props->can_move) {
        m_props->m_flags |= ImGuiWindowFlags_NoMove;
      }
      else {
        m_props->m_flags &= ~ImGuiWindowFlags_NoMove;
      }
    }
    ImGui::End();
  }

  void CameraSettingsWindow::drawCoordinates(float x, float y, float z) {
    ImGui::Text("X:");
    ImGui::SameLine();
    ImGui::Text("%.2f", x);
    ImGui::SameLine();
    ImGui::Text("Y:");
    ImGui::SameLine();
    ImGui::Text("%.2f", y);
    ImGui::SameLine();
    ImGui::Text("Z:");
    ImGui::SameLine();
    ImGui::Text("%.2f", z);
  }
}