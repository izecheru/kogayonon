#include "scene_viewport.h"

namespace kogayonon
{
  void SceneViewportWindow::draw()
  {
    ImGui::Begin(m_props->m_name.c_str(), nullptr, m_props->m_flags);
    ImGui::Text("ce mai faci?");
    ImGui::End();
  }
} // namespace kogayonon