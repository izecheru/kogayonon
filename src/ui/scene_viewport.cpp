#include "scene_viewport.h"

#include "context_manager/context_manager.h"

namespace kogayonon
{
void SceneViewportWindow::draw()
{
  m_fbo->bind();

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

  // we set this in renderer
  if (m_render_callback)
  {
    m_render_callback();
  }

  if (m_fbo->getHeight() != m_props->m_height || m_fbo->getWidth() != m_props->m_width)
  {
    ContextManager::klogger()->info("Viewport resized for Scene width:", m_props->m_width, " height:", m_props->m_height);
    m_fbo->setViewport(m_props->m_width, m_props->m_height);
    glViewport(0, 0, m_props->m_width, m_props->m_height);
  }
  ImVec2 win_pos = ImGui::GetCursorScreenPos();
  ImGui::GetWindowDrawList()->AddImage((void*)m_fbo->TEX(), ImVec2(win_pos.x, win_pos.y),
                                       ImVec2(win_pos.x + m_props->m_width, win_pos.y + m_props->m_height), ImVec2(0, 1), ImVec2(1, 0));

  ImGui::End();
  m_fbo->unbind();
}
} // namespace kogayonon