#include "gui/file_explorer.hpp"
#include <format>
#include <imgui_stdlib.h>
#include <spdlog/spdlog.h>
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/file_events.hpp"
#include "imgui_utils/imgui_utils.h"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/configurator/configurator.hpp"
#include "utilities/directory_watcher/directory_watcher.hpp"
#include "utilities/shader/shader_manager.hpp"

using namespace kogayonon_core;
using namespace kogayonon_utilities;

namespace kogayonon_gui
{
FileExplorerWindow::FileExplorerWindow( const std::string& name )
    : ImGuiWindow{ name }
    , m_update{ false }
    , m_currentPath{ std::filesystem::absolute( "." ) / "resources" }
    , m_pDirWatcher{ std::make_unique<DirectoryWatcher>( "resources\\" ) }
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
    m_pDispatcher->dispatchEvent<FileCreatedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileDeleted", [this]( const std::string& path, const std::string& name ) {
    FileDeletedEvent e( path, name );
    m_pDispatcher->dispatchEvent<FileDeletedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileRenamedNew", [this]( const std::string& path, const std::string& newName ) {
    FileRenamedEvent e( path, "", newName );
    m_pDispatcher->dispatchEvent<FileRenamedEvent>( e );
  } );

  m_pDirWatcher->setCommand( "fileModified", [this]( const std::string& path, const std::string& name ) {
    FileModifiedEvent e( path, name );
    m_pDispatcher->dispatchEvent<FileModifiedEvent>( e );
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
  auto& pShaderManager = MainRegistry::getInstance().getShaderManager();
  pShaderManager->markForRecompilation( e.getPath() );
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

void FileExplorerWindow::buildFileVector()
{
  m_files.clear();
  int dirId = 0;
  const auto& config = Configurator::getConfig();
  for ( auto const& dirEntry : std::filesystem::directory_iterator( m_currentPath ) )
  {
    bool found = false;
    if ( dirEntry.is_directory() )
    {
      for ( const auto& entry : config.folderFilters )
      {
        if ( dirEntry.path().filename().string().find( entry ) != std::string ::npos )
        {
          found = true;
          break;
        }
      }
    }
    else
    {
      for ( const auto& entry : config.fileFilters )
      {
        if ( dirEntry.path().extension().string().find( entry ) != std::string ::npos )
        {
          found = true;
          break;
        }
      }
    }

    if ( found )
      continue;

    File_ file{ .isDir = dirEntry.is_directory(),
                .imguiId = std::format( "{}{}", "##file", std::to_string( dirId ) ),
                .path = dirEntry.path() };
    dirId++;
    m_files.push_back( file );
  }
}

void FileExplorerWindow::drawFileContextMenu( const File_& file, const std::string& id )
{
  if ( ImGui::BeginPopupContextItem( id.c_str() ) )
  {
    if ( ImGui::MenuItem( "Delete file" ) )
    {
      std::filesystem::remove( file.path );
      spdlog::info( "removed {}", file.path.string() );
    }
    ImGui::EndPopup();
  }
}

void FileExplorerWindow::initIcons()
{
  static bool first{ false };

  if ( first )
    return;

  first = true;

  auto& assetManager = AssetManager::getInstance();
  m_icons.emplace( "file_default", assetManager.getTexture( "file.png" ).lock()->getTextureId() );
  m_icons.emplace( ".gltf", assetManager.getTexture( "gltf_icon.png" ).lock()->getTextureId() );
  m_icons.emplace( "folder", assetManager.getTexture( "folder.png" ).lock()->getTextureId() );
  m_icons.emplace( ".txt", assetManager.getTexture( "txt_icon.png" ).lock()->getTextureId() );
  m_icons.emplace( ".glsl", assetManager.getTexture( "shader_icon.png" ).lock()->getTextureId() );
  m_icons.emplace( ".png", assetManager.getTexture( "png_icon.png" ).lock()->getTextureId() );
  m_icons.emplace( ".jpg", assetManager.getTexture( "png_icon.png" ).lock()->getTextureId() );
  m_icons.emplace( ".jpeg", assetManager.getTexture( "png_icon.png" ).lock()->getTextureId() );
}

void FileExplorerWindow::draw()
{
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };

  if ( !begin() )
    return;

  initIcons();

  constexpr ImVec2 size{ 100.0f, 100.0f };
  constexpr float padding = 10.0f;
  float thumbnailSize = size.x;

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
    // since here we process file events we should recompile shaders here
    const auto& pShaderManager = MainRegistry::getInstance().getShaderManager();
    pShaderManager->compileMarkedShaders();

    lastPath = m_currentPath;
    buildFileVector();
    m_update.store( false );
  }

  ImGui::Columns( count, 0, false );
  for ( const auto& file : m_files )
  {
    if ( !file.renderable )
      continue;

    if ( file.isDir )
    {
      auto filename = file.path.filename();
      ImGui::BeginGroup();
      ImGui::ImageButton( file.imguiId.c_str(), (ImTextureID)m_icons.at( "folder" ), size );
      // navigate into folder like you do in windows explorer with double click
      if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
      {
        m_currentPath = file.path;
      }
      ImGui::TextWrapped( "%s", ImGui_Utils::truncateText( filename.stem().string(), size.x ).c_str() );
      ImGui::EndGroup();
    }
    else
    {
      auto filename = file.path.filename();
      ImGui::BeginGroup();
      drawFileContextMenu( file, filename.string() );
      // open shaders in the editor
      ImGui::ImageButton(
        file.imguiId.c_str(), (ImTextureID)fileExtensionToTextureId( file.path.extension().string() ), size );
      if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) &&
           file.path.extension().string() == ".glsl" )
      {
        // TODO add a setting for the user to choose his own editor
        ShellExecute( NULL, "open", "code", file.path.string().c_str(), NULL, SW_HIDE );
      }
      if ( ImGui::BeginDragDropSource( ImGuiDragDropFlags_SourceNoPreviewTooltip ) )
      {
        std::string path = file.path.string();
        ImGui::SetDragDropPayload( "ASSET_DROP", path.c_str(), path.size() + 1 );
        ImGui::EndDragDropSource();
      }
      ImGui::TextWrapped( "%s", ImGui_Utils::truncateText( filename.stem().string(), size.x ).c_str() );
      ImGui::EndGroup();
    }
    ImGui::NextColumn();
  }

  ImGui::End();
}

uint32_t FileExplorerWindow::fileExtensionToTextureId( const std::string& fileExtension )
{
  if ( !m_icons.contains( fileExtension ) )
    return m_icons.at( "file_default" );

  return m_icons.at( fileExtension );
}

void FileExplorerWindow::searchFor( const std::string& toFind )
{
  if ( toFind == "" )
  {
    for ( auto& file : m_files )
    {
      file.renderable = true;
    }
  }
  else
  {
    for ( auto& file : m_files )
    {
      if ( file.path.filename().string().find( toFind ) == std::string::npos )
      {
        file.renderable = false;
      }
      else
      {
        file.renderable = true;
      }
    }
  }
}

void FileExplorerWindow::drawToolbar()
{
  ImGui::BeginGroup();
  ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0, 0, 0, 0 } );
  ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, ImVec2{ 10.0f, 0.0f } );

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

  ImGui::SameLine();

  ImGui::SameLine();
  static std::string searchStr{ "" };
  if ( ImGui::InputText( "Search", &searchStr ) )
  {
    searchFor( searchStr );
  }

  ImGui::PopStyleVar( 1 );
  ImGui::PopStyleColor( 1 );
  ImGui::EndGroup();
}

} // namespace kogayonon_gui