#include "debug_window.h"

#include "registry_manager/registry_manager.h"

namespace kogayonon
{
void DebugConsoleWindow::clearLogs()
{
  KLogger::log(LogType::INFO, "Cleared logs in debug console window");
  if (m_messages.empty())
  {
    return;
  }
  m_messages.clear();
}

void DebugConsoleWindow::draw()
{
  if (!ImGui::Begin(m_props->m_name.c_str(), 0))
  {
    ImGui::End();
    return;
  }

  if (ImGui::Button("Clear"))
    clearLogs();

  ImGui::SameLine();
  ImGui::Checkbox("Auto-scroll", &m_auto_scroll);
  ImGui::Separator();

  ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

  for (auto& message : m_messages)
  {
    ImGui::TextUnformatted(message.c_str());
  }

  if (m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    ImGui::SetScrollHereY(1.0f);

  ImGui::EndChild();
  ImGui::End();
}
} // namespace kogayonon