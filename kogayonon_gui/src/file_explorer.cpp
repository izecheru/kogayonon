#include "gui/file_explorer.hpp"
#include <format>
#include <spdlog/spdlog.h>
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/file_events.hpp"
#include "imgui_utils/imgui_utils.h"
#include "utilities/directory_watcher/directory_watcher.hpp"

using namespace kogayonon_core;

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
    , m_pDispatcher{ std::make_unique<EventDispatcher>() }
{
  // installs the event listeners for file event types
  installHandlers();

  // installs the callback for the directory watcher
  // hotreloading textures or scripts and so on
  installCommands();
}

void FileExplorerWindow::installHandlers()
{
  m_pDispatcher->addHandler<FileCreatedEvent, &FileExplorerWindow::onFileCreated>( *this );
  m_pDispatcher->addHandler<FileRenamedEvent, &FileExplorerWindow::onFileRenamed>( *this );
  m_pDispatcher->addHandler<FileDeletedEvent, &FileExplorerWindow::onFileDeleted>( *this );
  m_pDispatcher->addHandler<FileModifiedEvent, &FileExplorerWindow::onFileModified>( *this );
}

void FileExplorerWindow::installCommands()
{
  m_pDirWatcher->setCommand( "fileCreated", [this]( const std::string& path, const std::string& name ) {
    FileCreatedEvent e( path, name );
    m_pDispatcher->emitEvent<FileCreatedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileDeleted", [this]( const std::string& path, const std::string& name ) {
    FileDeletedEvent e( path, name );
    m_pDispatcher->emitEvent<FileDeletedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileRenamedNew", [this]( const std::string& path, const std::string& newName ) {
    FileRenamedEvent e( path, "", newName );
    m_pDispatcher->emitEvent<FileRenamedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileModified", [this]( const std::string& path, const std::string& name ) {
    FileModifiedEvent e( path, name );
    m_pDispatcher->emitEvent<FileModifiedEvent>( e );
  } );
}

bool FileExplorerWindow::isTexture( const std::string& path )
{
  std::filesystem::path p{ path };
  auto ext = p.extension().string();
  return ext == ".jpg" || ext == ".png";
}

void FileExplorerWindow::onFileModified( FileModifiedEvent& e )
{
  if ( m_update == true )
    return;

  m_update.store( true );
}

void FileExplorerWindow::onFileCreated( FileCreatedEvent& e )
{
  if ( m_update == true )
    return;

  m_update.store( true );
}

void FileExplorerWindow::onFileDeleted( FileDeletedEvent& e )
{
  if ( m_update == true )
    return;

  m_update.store( true );
}

void FileExplorerWindow::onFileRenamed( FileRenamedEvent& e )
{
  if ( m_update == true )
    return;

  m_update.store( true );
}

void FileExplorerWindow::drawFileFilter()
{
  // TODO this should probably be written into some config file
  static auto filters = { ".bin", "fonts" };
  for ( const auto& filter : filters )
  {
    if ( ImGui::MenuItem( filter ) )
    {
    }
  }
}

void FileExplorerWindow::buildFileVector()
{
  m_files.clear();
  int dirId = 0;
  for ( auto const& dirEntry : std::filesystem::directory_iterator( m_currentPath ) )
  {
    // we don't want to see the "fonts" directory since we have no use for them rn
    if ( dirEntry.path().string().find( "fonts" ) != std::string::npos ||
         dirEntry.path().extension().string().find( ".bin" ) != std::string::npos )
      continue;

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

  if ( !begin() )
    return;

  static ImVec2 size{ 80.0f, 80.0f };
  static float padding = 20.0f;
  static float thumbnailSize = size.x;
  float cellSize = thumbnailSize + padding;
  float width = ImGui::GetContentRegionAvail().x;
  int count = width / cellSize;

  if ( count < 1 )
    count = 1;

  static std::filesystem::path lastPath{ "" };
  drawToolbar();

  // we build the vector of files only when we change the path or get a file event
  if ( m_update == true || m_currentPath != lastPath )
  {
    lastPath = m_currentPath;
    buildFileVector();
    m_update.store( false );
  }

  ImGui::Columns( count, 0, false );
  for ( const auto& file : m_files )
  {
    if ( file.isDir )
    {
      auto filename = file.path.filename();
      ImGui::BeginGroup();
      ImGui::ImageButton( file.imguiId.c_str(), (ImTextureID)m_folderTextureId, size );
      // navigate into folder like you do in windows explorer
      if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
      {
        m_currentPath = file.path;
      }
      ImGui::TextWrapped( "%s", filename.string().c_str() );
      ImGui::EndGroup();
    }
    else
    {
      auto filename = file.path.filename();
      ImGui::BeginGroup();
      ImGui::ImageButton( file.imguiId.c_str(), (ImTextureID)m_fileTextureId, size );
      if ( ImGui::BeginDragDropSource() )
      {
        std::string path = file.path.string();
        ImGui::SetDragDropPayload( "ASSET_DROP", path.c_str(), path.size() + 1 );
        ImGui::EndDragDropSource();
      }
      ImGui::TextWrapped( "%s", filename.string().c_str() );
      ImGui::EndGroup();
    }
    ImGui::NextColumn();
  }

  ImGui::End();
}

void FileExplorerWindow::drawToolbar()
{
  ImGui::BeginGroup();
  ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0, 0, 0, 0 ) );
  ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2( 10.0f, 0.0f ) );

  if ( static auto path = std::filesystem::current_path() / "resources"; m_currentPath == path )
  {
    // if we are on /resources directory disable it
    ImGui::BeginDisabled();
    ImGui::Button( "<" );
    ImGui::EndDisabled();
  }
  else
  {
    if ( ImGui::Button( "<" ) )
    {
      // go back to the parent dir
      m_currentPath = m_currentPath.parent_path();

      // build the file vector again
      buildFileVector();
    }
  }

  ImGui::PopStyleVar( 1 );
  ImGui::PopStyleColor( 1 );

  ImGui::SameLine();
  if ( ImGui::BeginCombo( "##filter_combo", "File filter" ) )
  {
    drawFileFilter();
    ImGui::EndCombo();
  }

  ImGui::EndGroup();
}
} // namespace kogayonon_gui