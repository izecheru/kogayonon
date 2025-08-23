#include "ui/win_camera_settings.h"

#include "context_manager/context_manager.h"
#include "renderer/camera.h"

namespace kogayonon
{
  void CameraSettingsWindow::draw()
  {
    auto& camera = ContextManager::camera();
    ImGui::Begin(m_props->m_name.c_str(), nullptr, m_props->m_flags);
    m_props->is_hovered = ImGui::IsWindowHovered();
    m_props->is_docked = ImGui::IsWindowDocked();
    drawProperties(camera->getProps());

    ImGui::SliderFloat("Mouse sensitivity", &camera->getProps().mouse_sens, 0.01f, 2.0f, "%.4f");
    ImGui::SliderFloat("Movement sensitivity", &camera->getProps().movement_speed, 0.01f, 100.0f, "%.4f");
    if (ImGui::Checkbox("Can move the window?", &m_props->can_move))
    {
      if (!m_props->can_move)
      {
        m_props->m_flags |= ImGuiWindowFlags_NoMove;
      }
      else
      {
        m_props->m_flags &= ~ImGuiWindowFlags_NoMove;
      }
    }
    ImGui::LabelText(m_props->is_docked == true ? "true" : "false", "Docked:");
    ImGui::End();
  }

  void CameraSettingsWindow::drawProperties(CameraProps& t_props)
  {
    ImGui::Text("X:");
    ImGui::SameLine();
    ImGui::Text("%.2f", t_props.position.x);
    ImGui::SameLine();
    ImGui::Text("Y:");
    ImGui::SameLine();
    ImGui::Text("%.2f", t_props.position.y);
    ImGui::SameLine();
    ImGui::Text("Z:");
    ImGui::SameLine();
    ImGui::Text("%.2f", t_props.position.z);

    ImGui::Text("Pitch:");
    ImGui::SameLine();
    ImGui::Text("%.2f", t_props.pitch);

    ImGui::Text("Yaw:");
    ImGui::SameLine();
    ImGui::Text("%.2f", t_props.yaw);
  }
} // namespace kogayonon