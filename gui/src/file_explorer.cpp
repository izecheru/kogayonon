#include "gui/imgui_windows/file_explorer.hpp"
#include <format>
#include <imgui_impl_vulkan.h>
#include <imgui_stdlib.h>
#include <spdlog/spdlog.h>
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/file_events.hpp"
#include "gui/utils/imgui_utils.hpp"
#include "utilities/configurator/configurator.hpp"
#include "utilities/directory_watcher/directory_watcher.hpp"
#include "utilities/fonts/fontawesome7.hpp"

using namespace core;
using namespace utilities;

namespace gui
{
FileExplorerWindow::FileExplorerWindow( const std::string& name, const FileExplorerSpec& spec )
    : ImGuiWindow{ name }
    , m_update{ false }
    , m_currentPath{ std::filesystem::absolute( "." ) / "engine_resources" }
    , m_pDirWatcher{ std::make_unique<DirectoryWatcher>( std::filesystem::absolute( "." ) ) }
    , m_pDispatcher{ std::make_unique<EventDispatcher>() }
    , m_spec{ spec }
{
  // installs the event listeners for file event types
  installHandlers();

  // installs the callback for the directory watcher
  // hotreloading textures or scripts and so on
  setCallback();
}

void FileExplorerWindow::installHandlers()
{
  m_pDispatcher->addHandler<FileEvent, &FileExplorerWindow::onFileEvent>( *this );
}

void FileExplorerWindow::setCallback()
{
  m_pDirWatcher->setCallback( [this]( const std::string& path, const std::string& name, const FileEventType& type ) {
    m_pDispatcher->dispatchEvent<FileEvent>( FileEvent{ path, name, type } );
  } );
}

bool FileExplorerWindow::isTexture( const std::string& path )
{
  std::filesystem::path p{ path };
  auto ext = p.extension().string();
  return ext == ".jpg" || ext == ".png";
}

void FileExplorerWindow::onFileEvent( FileEvent& e )
{
  if ( m_update == true )
    return;

  // handle all modify cases, currently we just do hot-load for shaders
  switch ( e.getType() )
  {
    // handle all the modify functionality
  case FileEventType::Modify: {
    if ( e.getPath().find( "glsl" ) != std::string::npos )
    {
      auto& pShaderManager = MainRegistry::getInstance().getShaderManager();
    }
    if ( e.getPath().find( "config" ) != std::string::npos )
    {
      // rebuild the file vector, filters might have changed in config file
      buildFileVector();
    }
  }
  break;
  case FileEventType::Create: {
  }
  break;
  case FileEventType::Rename: {
  }
  break;
  case FileEventType::Delete: {
  }
  break;
  default:
    spdlog::error( "something went wrong, enum FileEventType does not support this value" );
    break;
  }

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

void FileExplorerWindow::render()
{
  if ( !begin() )
    return;

  static ImVec2 size{ 100.0f, 100.0f };
  static float padding = 12.0f;
  static float thumbnailSize = size.x;
  static float cellSize = thumbnailSize + ( 2 * padding );

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

  ImGui::BeginTable( "##fileTable", count, ImGuiTableFlags_NoPadOuterX );
  for ( const auto& file : m_files )
  {
    if ( !file.renderable )
      continue;

    ImGui::TableNextColumn();

    if ( file.isDir )
    {
      auto filename = file.path.filename();
      ImGui::BeginGroup();
      ImGui::ImageButton( file.imguiId.c_str(), (ImTextureID)m_spec.folderIcon, size );
      //  navigate into folder like you do in windows explorer with double click
      if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
      {
        m_currentPath = file.path;
      }
      ImGui::TextWrapped( "%s", gui_utils::truncateText( filename.stem().string(), size.x ).c_str() );
      ImGui::EndGroup();
    }
    else
    {
      auto filename = file.path.filename();
      ImGui::BeginGroup();
      drawFileContextMenu( file, filename.string() );
      // open shaders in the editor
      ImGui::ImageButton( file.imguiId.c_str(), (ImTextureID)m_spec.fileIcon, size );
      if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) &&
           file.path.extension().string() == ".glsl" )
      {
        // TODO(kogayonon) add a setting for the user to choose his own editor
        ShellExecute( NULL, "open", "code", file.path.string().c_str(), NULL, SW_HIDE );
      }
      if ( ImGui::BeginDragDropSource( ImGuiDragDropFlags_SourceNoPreviewTooltip ) )
      {
        std::string path = file.path.string();
        ImGui::SetDragDropPayload( "ASSET_DROP", path.c_str(), path.size() + 1 );
        ImGui::EndDragDropSource();
      }
      ImGui::TextWrapped( "%s", gui_utils::truncateText( filename.stem().string(), size.x ).c_str() );
      ImGui::EndGroup();
    }
  }
  ImGui::EndTable();
  ImGui::End();
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
      auto filename = file.path.filename().stem().string();
      if ( filename.find( toFind ) != std::string::npos )
      {
        file.renderable = true;
      }
      else
      {
        file.renderable = false;
      }
    }
  }
}

void FileExplorerWindow::drawToolbar()
{
  ImGui::BeginGroup();
  ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4{ 0, 0, 0, 0 } );

  static std::string searchStr{ "" };
  ImGui::Text( ICON_FA_SEARCH " " );
  ImGui::SameLine();
  if ( ImGui::InputText( "##searchId", &searchStr ) )
  {
    searchFor( searchStr );
  }

  if ( m_currentPath != std::filesystem::current_path() / "engine_resources" )
  {
    std::filesystem::path p = m_currentPath;
    std::vector<std::filesystem::path> folders;
    while ( p != std::filesystem::current_path() )
    {
      folders.emplace_back( p );
      p = p.parent_path();
    }

    for ( auto it = folders.rbegin(); it != folders.rend(); it++ )
    {
      if ( *it == folders.at( 0 ) )
      {
        RenderDisabled( ImGui::Button( it->filename().string().c_str() ); )
      }
      else
      {
        if ( ImGui::Button( it->filename().string().c_str() ) )
        {
          m_currentPath = *it;
          buildFileVector();
        }
      }
      ImGui::SameLine( 0.0f, 4.0f );
    }
  }
  else
  {
    RenderDisabled( ImGui::Button( m_currentPath.filename().string().c_str() ); )
  }

  ImGui::PopStyleColor();
  ImGui::EndGroup();
}

} // namespace gui
