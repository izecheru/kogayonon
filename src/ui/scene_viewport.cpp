#include "scene_viewport.h"

namespace kogayonon
{
void SceneViewportWindow::draw()
{
  if (!ImGui::Begin(m_props->m_name.c_str(), nullptr, m_props->m_flags))
  {
    ImGui::End();
    return;
  }
  ImGui::Text("ce mai faci?");
  ImGui::End();
}
} // namespace kogayonon