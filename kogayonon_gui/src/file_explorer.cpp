#include "gui/file_explorer.hpp"


#include <spdlog/spdlog.h>
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/file_events.hpp"
#include "imgui_utils/imgui_utils.h"
#include "utilities/directory_watcher/directory_watcher.hpp"

namespace kogayonon_gui
{
kogayonon_gui::FileExplorerWindow::FileExplorerWindow( std::string name, uint32_t folderTextureId,
                                                       uint32_t fileTextureId )
    : ImGuiWindow{ std::move( name ) }
    , m_update{ false }
    , m_folderTextureId{ folderTextureId }
    , m_fileTextureId{ fileTextureId }
    , m_currentPath{ std::filesystem::current_path() / "resources" }
    , m_pDirWatcher{ std::make_unique<kogayonon_utilities::DirectoryWatcher>( "resources\\" ) }
    , m_pDispatcher{ std::make_unique<kogayonon_core::EventDispatcher>() }
{
  // installs the event listeners for file event types
  installHandlers();

  // installs the callback for the directory watcher
  // hotreloading textures or scripts and so on
  installCommands();
}

void FileExplorerWindow::installHandlers()
{
  m_pDispatcher->addHandler<kogayonon_core::FileCreatedEvent, &FileExplorerWindow::onFileCreated>( *this );
  m_pDispatcher->addHandler<kogayonon_core::FileRenamedEvent, &FileExplorerWindow::onFileRenamed>( *this );
  m_pDispatcher->addHandler<kogayonon_core::FileDeletedEvent, &FileExplorerWindow::onFileDeleted>( *this );
  m_pDispatcher->addHandler<kogayonon_core::FileModifiedEvent, &FileExplorerWindow::onFileModified>( *this );
}

void FileExplorerWindow::installCommands()
{
  m_pDirWatcher->setCommand( "fileCreated", [this]( std::string path, std::string name ) {
    kogayonon_core::FileCreatedEvent e( path, name );
    m_pDispatcher->emitEvent<kogayonon_core::FileCreatedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileDeleted", [this]( std::string path, std::string name ) {
    kogayonon_core::FileDeletedEvent e( path, name );
    m_pDispatcher->emitEvent<kogayonon_core::FileDeletedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileRenamedNew", [this]( std::string path, std::string newName ) {
    kogayonon_core::FileRenamedEvent e( path, "", newName );
    m_pDispatcher->emitEvent<kogayonon_core::FileRenamedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileModified", [this]( std::string path, std::string name ) {
    kogayonon_core::FileModifiedEvent e( path, name );
    m_pDispatcher->emitEvent<kogayonon_core::FileModifiedEvent>( e );
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
  if ( m_update == true )
    return;

  m_update.store( true );
}

void FileExplorerWindow::onFileCreated( kogayonon_core::FileCreatedEvent& e )
{
  if ( m_update == true )
    return;

  m_update.store( true );
}

void FileExplorerWindow::onFileDeleted( kogayonon_core::FileDeletedEvent& e )
{
  if ( m_update == true )
    return;

  m_update.store( true );
}

void FileExplorerWindow::onFileRenamed( kogayonon_core::FileRenamedEvent& e )
{
  if ( m_update == true )
    return;

  m_update.store( true );
}

void FileExplorerWindow::draw()
{
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };
  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    ImGui::End();
    return;
  }


    ImGui::SameLine();
  }

  ImGui::End();
}

void FileExplorerWindow::drawPathToolbar()
{

  ImGui::PopStyleVar( 1 );
  ImGui::PopStyleColor( 1 );

  ImGui::EndGroup();

}
} // namespace kogayonon_gui