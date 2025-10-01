#include "gui/file_explorer.hpp"
#include <format>
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
  m_pDirWatcher->setCommand( "fileCreated", [this]( const std::string& path, const std::string& name ) {
    kogayonon_core::FileCreatedEvent e( path, name );
    m_pDispatcher->emitEvent<kogayonon_core::FileCreatedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileDeleted", [this]( const std::string& path, const std::string& name ) {
    kogayonon_core::FileDeletedEvent e( path, name );
    m_pDispatcher->emitEvent<kogayonon_core::FileDeletedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileRenamedNew", [this]( const std::string& path, const std::string& newName ) {
    kogayonon_core::FileRenamedEvent e( path, "", newName );
    m_pDispatcher->emitEvent<kogayonon_core::FileRenamedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileModified", [this]( const std::string& path, const std::string& name ) {
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

void FileExplorerWindow::buildFileVector()
{
  m_files.clear();
  int dirId = 0;
  for ( auto const& dirEntry : std::filesystem::directory_iterator( m_currentPath ) )
  {
    // we don't want to see the "fonts" directory since we have no use for them rn
    if ( dirEntry.path().string().find( "fonts" ) != std::string::npos )
      continue;

    // TODO add filters here
    File_ file{ .isDir = dirEntry.is_directory(),
                .imguiId = std::format( "{}{}", "##file", std::to_string( dirId ) ),
                .path = dirEntry.path() };
    dirId++;
    m_files.emplace_back( file );
  }
}

void FileExplorerWindow::draw()
{
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };
  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    ImGui::End();
    return;
  }

  static std::filesystem::path lastPath{ "" };
  drawPathToolbar();

  // we build the vector of files only when we change the path or get a file event
  if ( m_update == true || m_currentPath != lastPath )
  {
    lastPath = m_currentPath;
    buildFileVector();
    m_update.store( false );
  }

  // we only want 10 files per line
  int count = 0;
  for ( const auto& file : m_files )
  {
    ++count;
    if ( file.isDir )
    {
      auto filename = file.path.filename();
      ImGui::BeginGroup();
      if ( ImGui::ImageButton( file.imguiId.c_str(), (ImTextureID)m_folderTextureId, ImVec2{ 100.0f, 100.0f } ) )
      {
        m_currentPath = file.path;
      }
      ImGui::Text( "%s", ImGui_Utils::truncateText( filename.string(), 100.0f ).c_str() );
      ImGui::EndGroup();
    }
    else
    {
      auto filename = file.path.filename();
      ImGui::BeginGroup();
      ImGui::ImageButton( file.imguiId.c_str(), (ImTextureID)m_fileTextureId, ImVec2{ 100.0f, 100.0f } );
      if ( ImGui::BeginDragDropSource() )
      {
        std::string path = file.path.string();
        ImGui::SetDragDropPayload( "ASSET_DROP", path.c_str(), path.size() );
        ImGui::EndDragDropSource();
      }
      ImGui::Text( "%s", ImGui_Utils::truncateText( filename.string(), 100.0f ).c_str() );
      ImGui::EndGroup();
    }

    if ( count != 10 )
      ImGui::SameLine();

    if ( count == 10 )
      count = 0;
  }

  ImGui::End();
}

void FileExplorerWindow::drawPathToolbar()
{
  ImGui::BeginGroup();
  ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0, 0, 0, 0 ) );
  ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 10.0f, 0.0f ) );

  if ( static auto path = std::filesystem::current_path() / "resources"; m_currentPath == path )
  {
    ImGui::BeginDisabled();
    ImGui::Button( "<" );
    ImGui::EndDisabled();
  }
  else
  {
    if ( ImGui::Button( "<" ) )
    {
      m_currentPath = m_currentPath.parent_path();
      buildFileVector();
    }
  }

  ImGui::PopStyleVar( 1 );
  ImGui::PopStyleColor( 1 );

  ImGui::EndGroup();
}
} // namespace kogayonon_gui