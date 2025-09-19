#include "gui/performance_window.hpp"
#include <chrono>
#include "core/ecs/main_registry.hpp"
#include "utilities/time_tracker/time_tracker.hpp"

namespace kogayonon_gui
{
// could also get more stats into this window like draw calls and so on
PerformanceWindow::PerformanceWindow( std::string name )
    : ImGuiWindow( std::move( name ) )
{
}

void PerformanceWindow::draw()
{
  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    ImGui::End();
    return;
  }
  static std::chrono::high_resolution_clock::time_point initial = std::chrono::high_resolution_clock::now();

  auto passed = std::chrono::high_resolution_clock::now();
  ImGui::Text( "FPS:" );
  ImGui::SameLine();
  static int fps = static_cast<int>( 1.0f / TIME_TRACKER()->getDuration( "deltaTime" ).count() );
  static double frameTimeMilli =
    std::chrono::duration<double, std::milli>( TIME_TRACKER()->getDuration( "deltaTime" ) ).count();

  if ( std::chrono::duration<double>( passed - initial ).count() > 1.0f )
  {
    initial = std::chrono::high_resolution_clock::now();
    fps = static_cast<int>( 1.0f / TIME_TRACKER()->getDuration( "deltaTime" ).count() );
    frameTimeMilli = std::chrono::duration<double, std::milli>( TIME_TRACKER()->getDuration( "deltaTime" ) ).count();
  }

  ImGui::Text( "%d", fps );
  ImGui::Text( "Frame time %.3f ms", frameTimeMilli );

  ImGui::End();
}
} // namespace kogayonon_gui