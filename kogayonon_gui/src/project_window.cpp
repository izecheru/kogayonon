#include "gui/project_window.hpp"
#include <fstream>
#include <spdlog/spdlog.h>
#include <tinyfiledialogs/tinyfiledialogs.h>
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/project_event.hpp"
#include "utilities/asset_manager/asset_manager.hpp"

namespace kogayonon_gui
{
ProjectWindow::ProjectWindow( const std::string& name )
    : ImGuiWindow{ name }
{
}

void ProjectWindow::draw()
{
  if ( !begin() )
    return;

  auto& assetManager = kogayonon_utilities::AssetManager::getInstance();
  const auto& pEventDispatcher = kogayonon_core::MainRegistry::getInstance().getEventDispatcher();

  static auto enginePictureTexture = assetManager.getTexture( "logo.png" ).lock();
  static auto max = ImGui::GetContentRegionMax();
  ImGui::Image( (ImTextureID)enginePictureTexture->getTextureId(), max );

  ImGui::SetCursorPos( ImVec2{ 50.0f, m_props->height - 50.0f } );
  ImGui::CalcItemWidth();
  auto path = std::filesystem::absolute( "projects\\" ).string();

  // if the directory does not exist, create it
  if ( !std::filesystem::exists( path ) )
  {
    std::filesystem::create_directories( path );
  }

  const char* filters[] = { "*.kproj" };

  if ( ImGui::Button( "Open project" ) )
  {
    auto result = tinyfd_openFileDialog( "Open project", path.c_str(), 1, filters, ".kproj file", false );

    if ( result )
    {
      hide();
      spdlog::info( "Project opened {}", result );
      pEventDispatcher->emitEvent( kogayonon_core::ProjectLoadEvent{ result } );
    }
  }

  // we need to push the button to the left with textSize + frame padding .x *2 + 50.0f
  auto textSize = ImGui::CalcTextSize( "New project" );
  float toPush = textSize.x + ( ImGui::GetStyle().FramePadding.x * 2 ) + 50.0f;
  ImGui::SetCursorPos( ImVec2{ m_props->width - toPush, m_props->height - 50.0f } );

  if ( ImGui::Button( "New project" ) )
  {
    auto result = tinyfd_saveFileDialog( "Save project", path.c_str(), 1, filters, ".kproj file" );

    if ( result )
    {
      std::ofstream writeProjFile{ result };
      if ( writeProjFile.is_open() )
      {
        hide();
        writeProjFile.close();
        pEventDispatcher->emitEvent( kogayonon_core::ProjectCreateEvent{ result } );
      }
      else
      {
        spdlog::critical( "CANNOT CREATE PROJECT FILE" );
      }
    }
  }

  end();
}
} // namespace kogayonon_gui