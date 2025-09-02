#include "file_explorer.h"

namespace kogayonon
{
void FileExplorerWindow::draw()
{
  if (!ImGui::Begin(m_props->m_name.c_str(), nullptr, m_props->flags))
  {
    return;
  }

  for (auto& it : m_displays)
  {
    it.second->draw();
  }

  ImGui::End();
}
} // namespace kogayonon