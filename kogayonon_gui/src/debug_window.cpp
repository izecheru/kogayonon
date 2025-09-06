#include "gui/debug_window.h"

namespace kogayonon_gui {
void DebugConsoleWindow::clearLogs()
{
    if ( m_messages.empty() )
    {
        return;
    }
    m_messages.clear();
    Logger::log( LogType::INFO, "Cleared logs in debug console window" );
}

void DebugConsoleWindow::draw()
{
    if ( !ImGui::Begin( m_props->m_name.c_str(), 0 ) )
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
        for ( auto& message : m_messages )
        {
            if ( message.find( "critical" ) != std::string::npos )
            {
                ImGui::TextColored( logTypeToColor( LogType::CRITICAL ), message.c_str() );
            }
            else if ( message.find( "error" ) != std::string::npos )
            {
                ImGui::TextColored( logTypeToColor( LogType::ERROR ), message.c_str() );
            }
            else if ( message.find( "info" ) != std::string::npos )
            {
                ImGui::TextColored( logTypeToColor( LogType::INFO ), message.c_str() );
            }
            else if ( message.find( "debug" ) != std::string::npos )
            {
                ImGui::TextColored( logTypeToColor( LogType::DEBUG ), message.c_str() );
            }
        }

        if ( m_auto_scroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() )
            ImGui::SetScrollHereY( 1.0f );
    }
    ImGui::EndChild();
    ImGui::End();
}
} // namespace kogayonon_gui