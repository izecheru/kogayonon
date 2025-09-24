#include "gui/file_explorer.hpp"
#include <spdlog/spdlog.h>
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/file_events.hpp"
#include "imgui_utils/imgui_utils.h"
#include "utilities/directory_watcher/directory_watcher.hpp"

namespace kogayonon_gui
{
FileExplorerWindow::FileExplorerWindow( std::string name, unsigned int folderTextureId, unsigned int fileTextureId )
    : ImGuiWindow{ std::move( name ) }
    , m_currentPath{ std::filesystem::current_path() / "resources" }
    , m_pDirWatcher{ std::make_unique<kogayonon_utilities::DirectoryWatcher>( "resources\\" ) }
    , m_fileTextureId{ fileTextureId }
    , m_folderTextureId{ folderTextureId }
{
  // installs the event listeners for file event types
  installHandlers();

  // installs the callback for the directory watcher
  // hotreloading textures or scripts and so on
  installCommands();
}

void FileExplorerWindow::installHandlers()
{
  EVENT_DISPATCHER()->addHandler<kogayonon_core::FileCreatedEvent, &FileExplorerWindow::onFileCreated>( *this );
  EVENT_DISPATCHER()->addHandler<kogayonon_core::FileRenamedEvent, &FileExplorerWindow::onFileRenamed>( *this );
  EVENT_DISPATCHER()->addHandler<kogayonon_core::FileDeletedEvent, &FileExplorerWindow::onFileDeleted>( *this );
  EVENT_DISPATCHER()->addHandler<kogayonon_core::FileModifiedEvent, &FileExplorerWindow::onFileModified>( *this );
}

void FileExplorerWindow::installCommands()
{
  m_pDirWatcher->setCommand( "fileCreated", [dispatcher = EVENT_DISPATCHER()]( std::string path, std::string name ) {
    kogayonon_core::FileCreatedEvent e( path, name );
    dispatcher->emitEvent<kogayonon_core::FileCreatedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileDeleted", [dispatcher = EVENT_DISPATCHER()]( std::string path, std::string name ) {
    kogayonon_core::FileDeletedEvent e( path, name );
    dispatcher->emitEvent<kogayonon_core::FileDeletedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileRenamedNew",
                             [dispatcher = EVENT_DISPATCHER()]( std::string path, std::string newName ) {
                               kogayonon_core::FileRenamedEvent e( path, "", newName );
                               dispatcher->emitEvent<kogayonon_core::FileRenamedEvent>( e );
                             } );

  m_pDirWatcher->setCommand( "fileModified", [dispatcher = EVENT_DISPATCHER()]( std::string path, std::string name ) {
    kogayonon_core::FileModifiedEvent e( path, name );
    dispatcher->emitEvent<kogayonon_core::FileModifiedEvent>( e );
  } );
}

bool FileExplorerWindow::isTexture( const std::string& path )
{
  std::filesystem::path p{ path };
  auto ext = p.extension().string();
  return ext == ".jpg" || ext == ".png";
}

void FileExplorerWindow::onFileModified( kogayonon_core::FileModifiedEvent& e )
{
  spdlog::info( "FILE MODIFIED {} name {}", e.getPath(), e.getName() );
}

void FileExplorerWindow::onFileCreated( kogayonon_core::FileCreatedEvent& e )
{
  spdlog::info( "FILE CREATED {} name {}", e.getPath(), e.getName() );
}

void FileExplorerWindow::onFileDeleted( kogayonon_core::FileDeletedEvent& e )
{
  spdlog::info( "FILE DELETED {} name {}", e.getPath(), e.getName() );
}

void FileExplorerWindow::onFileRenamed( kogayonon_core::FileRenamedEvent& e )
{
  spdlog::info( "FILE RENAMED {} name {}", e.getPath(), e.getName() );
}

void FileExplorerWindow::draw()
{
  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    ImGui::End();
    return;
  }

  // draw the path toolbar used for navigation
  drawPathToolbar();

  // for each imgui element we need an id that is unique, so I use this dirId to add it to already defined const char*
  int dirId = 0;

  // for easier entry size modifications
  static ImVec2 entrySize = ImVec2( 100.f, 100.f );

  for ( auto const& dirEntry : std::filesystem::directory_iterator( m_currentPath ) )
  {
    std::string id = "##file" + std::to_string( dirId++ );
    const auto& path = dirEntry.path();
    auto relativePath = std::filesystem::relative( path );

    if ( dirEntry.is_directory() )
    {
      // we don't want to see the "fonts" directory since we have no use for them rn
      if ( path.string().find( "fonts" ) != std::string::npos )
      {
        continue;
      }

      ImVec2 textSize = ImGui::CalcTextSize( path.filename().string().c_str() );
      ImGui::BeginGroup();

      if ( ImGui::ImageButton( id.c_str(), (ImTextureID)m_folderTextureId, entrySize ) )
      {
        m_currentPath = path;
      }

      ImGui::Text( "%s", ImGui_Utils::truncateText( path.filename().string(), entrySize.x ).c_str() );
      ImGui::EndGroup();
    }
    else
    {
      // if it is a file
      ImGui::BeginGroup();

      ImGui::ImageButton( id.c_str(), (ImTextureID)m_fileTextureId, entrySize );

      // only files can be dragged and dropped, will add filters later so we don't even see .zip files for example
      if ( ImGui::BeginDragDropSource() )
      {
        std::string test = relativePath.string();
        ImGui::SetDragDropPayload( "ASSET_DROP", test.c_str(), test.size(), ImGuiCond_Once );
        ImGui::EndDragDropSource();
      }

      ImVec2 fileNameSize = ImGui::CalcTextSize( path.filename().string().c_str() );
      ImGui::Text( "%s", ImGui_Utils::truncateText( path.filename().string(), entrySize.x ).c_str() );
      ImGui::EndGroup();
      if ( ImGui::IsItemHovered() )
      {
        ImGui::BeginTooltip();
        ImGui::Text( path.filename().string().c_str() );
        ImGui::EndTooltip();
      }
    }

    ImGui::SameLine();
  }

  ImGui::End();
}

void FileExplorerWindow::drawPathToolbar()
{
  std::vector<std::string> pathItems;
  // construct the path from D:/folder1/folder to to ["D","folde1","folder2"] for easier access
  if ( m_currentPath != std::filesystem::current_path() )
  {
    std::stringstream ss( m_currentPath.string() );
    std::string item;
    bool resources = false;
    while ( std::getline( ss, item, '\\' ) )
    {
      if ( !item.empty() )
      {
        // we only push back items right of resources (included)
        if ( item == "resources" )
        {
          resources = true;
        }

        if ( resources )
          pathItems.push_back( item );
      }
    }
  }

  static int currentIndex = -1;

  ImGui::BeginGroup();
  ImVec2 pathSizeText = ImGui::CalcTextSize( "Current path" );
  auto cursor = ImGui::GetCursorPos();
  // ImGui::SetCursorPos( ImVec2( cursor.x + 10.0f, cursor.y + 10.0f ) );
  ImGui::Text( "Current path", ImVec2( pathSizeText.x + 20.0f, pathSizeText.y + 10.0f ) );

  ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0, 0, 0, 0 ) );
  ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 10.0f, 0.0f ) );

  for ( int i = 0; i < pathItems.size(); ++i )
  {
    ImGui::SameLine( 0.0f, 10.0f );
    // if we press on a folder from path toolbar
    ImVec2 textSize = ImGui::CalcTextSize( pathItems.at( i ).c_str() );

    if ( ImGui::Button( pathItems.at( i ).c_str() ) )
    {
      // don't set currentIndex if it is already clicked once
      if ( currentIndex != i )
      {
        // we get the index of the button we pressed
        currentIndex = i;
        break;
      }
    }
    ImGui::SameLine();

    // if we are on resources/  don't render the arrrow
    // if we are at the last entry in the pathItems vec, also don't render cause we're pointing to nothing
    if ( pathItems.size() > 1 && i != pathItems.size() - 1 )
      ImGui::Text( "->", ImVec2( 20.0f, 20.0f ) );
  }
  ImGui::PopStyleVar( 1 );
  ImGui::PopStyleColor( 1 );

  ImGui::EndGroup();

  // if we did not press any button
  if ( currentIndex != -1 )
  {
    std::filesystem::path result;
    // from start to the index we got, we construct the path and update m_currentPath
    for ( int i = 0; i <= currentIndex; i++ )
    {
      result /= pathItems.at( i );
    }
    m_currentPath = std::filesystem::current_path() / result;
    currentIndex = -1;
  }
}
} // namespace kogayonon_gui