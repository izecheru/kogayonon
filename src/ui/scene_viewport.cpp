#include "scene_viewport.h"

#include "context_manager/context_manager.h"

namespace kogayonon
{
void SceneViewportWindow::draw()
{
  if (!ImGui::Begin(m_props->m_name.c_str(), nullptr, m_props->m_flags))
  {
    ImGui::End();
    return;
  }
  m_props->m_width = ImGui::GetContentRegionAvail().x;
  m_props->m_height = ImGui::GetContentRegionAvail().y;
  glViewport(0, 0, m_fbo->getWidth(), m_fbo->getHeight());
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  int fbWidth = (int)m_props->m_width;
  int fbHeight = (int)m_props->m_height;
  m_fbo->rescaleFramebuffer(fbWidth, fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_fbo->bind(); // we set this in renderer
  if (m_render_callback)
  {
    m_render_callback();
  }
  ImVec2 win_pos = ImGui::GetCursorScreenPos();

  ImGui::GetWindowDrawList()->AddImage((void*)m_fbo->getTexture(), ImVec2(win_pos.x, win_pos.y),
                                       ImVec2(win_pos.x + m_props->m_width, win_pos.y + m_props->m_height), ImVec2(0, 1), ImVec2(1, 0));
  ImGui::End();
  m_fbo->unbind();
}
} // namespace kogayonon