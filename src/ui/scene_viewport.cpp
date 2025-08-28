#include "scene_viewport.h"

namespace kogayonon
{
void SceneViewportWindow::draw()
{
  if (!ImGui::Begin(m_props->m_name.c_str(), nullptr, m_props->flags))
  {
    ImGui::End();
    return;
  }
  m_props->width = ImGui::GetContentRegionAvail().x;
  m_props->height = ImGui::GetContentRegionAvail().y;

  // draw to framebuffer
  int fbWidth = (int)m_props->width;
  int fbHeight = (int)m_props->height;
  m_fbo->bind();
  m_fbo->rescaleFramebuffer(fbWidth, fbHeight);
  glViewport(0, 0, fbWidth, fbHeight);
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (m_render_callback)
  {
    m_render_callback();
  }
  m_fbo->unbind();

  ImVec2 win_pos = ImGui::GetCursorScreenPos();

  ImGui::GetWindowDrawList()->AddImage((void*)m_fbo->getTexture(), ImVec2(win_pos.x, win_pos.y),
                                       ImVec2(win_pos.x + m_props->width, win_pos.y + m_props->height), ImVec2(0, 1), ImVec2(1, 0));
  ImGui::End();
}

} // namespace kogayonon