#include "gui/debug_window.hpp"
#include "imgui_utils/imgui_utils.h"

namespace kogayonon_gui
{
void DebugConsoleWindow::clearLogs()
{
  if ( m_messages.empty() )
  {
    return;
  }
  m_messages.clear();
}

void DebugConsoleWindow::draw()
{
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };
  if ( !ImGui::Begin( m_props->name.c_str(), 0 ) )
  {
    ImGui::End();
    return;
  }

  if ( ImGui::Button( "Clear" ) )
    clearLogs();

  ImGui::SameLine();
  ImGui::Checkbox( "Auto-scroll", &m_auto_scroll );
  ImGui::Separator();

  if ( ImGui::BeginChild( "ScrollingRegion", ImVec2( 0, 0 ), false, ImGuiWindowFlags_HorizontalScrollbar ) )
  {
    for ( const auto& message : m_messages )
    {
      ImGui::Text( message.c_str() );
    }

    if ( m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() )
      ImGui::SetScrollHereY( 1.0f );
  }
  ImGui::EndChild();
  ImGui::End();
}
} // namespace kogayonon_gui