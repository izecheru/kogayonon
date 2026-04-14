#include "gui/imgui_windows/file_explorer.hpp"
#include <format>
#include <imgui_impl_vulkan.h>
#include <imgui_stdlib.h>
#include <spdlog/spdlog.h>
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/file_events.hpp"
#include "gui/utils/imgui_utils.hpp"
#include "utilities/config_manager/config_manager.hpp"
#include "utilities/directory_watcher/directory_watcher.hpp"
#include "utilities/fonts/materialdesign.hpp"

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
    , m_searchStr{ "" }
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
  const auto& config = EditorConfigManager::getConfig();
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

  constexpr ImVec2 size{ 110.0f, 110.0f };
  constexpr ImVec2 childSize{ 120.0f, 120.0f };
  constexpr float padding = 10.0f;
  float thumbnailSize = size.x;

  float cellSize = thumbnailSize + ( 2 * padding );
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

  ImGui::BeginChild( "##fileExplorerScrollRegion" );
  ImGui::BeginTable( "##fileExplorerTable", count );
  for ( const auto& file : m_files )
  {
    if ( !file.renderable )
      continue;

    ImGui::PushStyleVar( ImGuiStyleVar_CellPadding, ImVec2{ 0.0f, 10.0f } );
    ImGui::PushItemWidth( childSize.x );
    ImGui::TableNextColumn();
    if ( file.isDir )
    {
      auto pos = ImGui::GetCursorScreenPos();
      auto textHeight = ImGui::CalcTextSize( file.path.filename().stem().string().c_str() );
      ImGui::GetWindowDrawList()->AddRectFilled(
        pos, ImVec2{ pos.x + childSize.x, pos.y + childSize.y + textHeight.y }, IM_COL32( 87, 91, 180, 70 ) );

      ImGui::BeginGroup();
      auto filename = file.path.filename();
      ImGui::SetCursorPosX( ImGui::GetCursorPosX() + 5.0f );
      ImGui::Image( m_spec.iconGenericFolder, size );

      auto truncatedText = gui_utils::truncateText( filename.stem().string(), size.x );
      gui_utils::moveTextToCenter( size, truncatedText );
      RenderWithSizedFont( m_spec.fonts->at( "inter" ), 20.0f, ImGui::TextWrapped( "%s", truncatedText.c_str() ) );
      // navigate into folder like you do in windows explorer with double click
      ImGui::EndGroup();
      if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) )
      {
        m_currentPath = file.path;
        m_searchStr = "";
      }
    }
    else
    {
      auto filename = file.path.filename();
      auto pos = ImGui::GetCursorScreenPos();
      auto textHeight = ImGui::CalcTextSize( file.path.filename().stem().string().c_str() );
      ImGui::GetWindowDrawList()->AddRectFilled(
        pos, ImVec2{ pos.x + childSize.x, pos.y + childSize.y + textHeight.y }, IM_COL32( 87, 91, 180, 70 ) );

      ImGui::BeginGroup();
      ImGui::SetCursorPosX( ImGui::GetCursorPosX() + 5.0f );
      ImGui::Image( fileTexture( file ), size );
      auto truncatedText = gui_utils::truncateText( filename.stem().string(), size.x );
      gui_utils::moveTextToCenter( size, truncatedText );
      RenderWithSizedFont( m_spec.fonts->at( "inter" ), 20.0f, ImGui::TextWrapped( "%s", truncatedText.c_str() ) );
      ImGui::EndGroup();

      drawFileContextMenu( file, filename.string() );
      if ( ImGui::BeginDragDropSource( ImGuiDragDropFlags_SourceNoPreviewTooltip |
                                       ImGuiDragDropFlags_SourceAllowNullID ) )
      {
        std::string path = file.path.string();
        ImGui::SetDragDropPayload( "ASSET_DROP", path.c_str(), path.size() + 1 );
        ImGui::EndDragDropSource();
      }

      if ( ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) &&
           file.path.extension().string() == ".glsl" )
      {
        // TODO(kogayonon) add a setting for the user to choose his own editor
        ShellExecute( NULL, "open", "code", file.path.string().c_str(), NULL, SW_HIDE );
      }
    }
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
  }
  ImGui::EndTable();
  ImGui::EndChild();
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

  ImGui::Text( ICON_MDI_FILE_SEARCH "" );
  ImGui::SameLine();
  ImGui::PushItemWidth( 200.0f );
  if ( ImGui::InputText( "##searchId", &m_searchStr ) )
  {
    searchFor( m_searchStr );
  }
  ImGui::PopItemWidth();

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
          m_searchStr = "";
          buildFileVector();
        }
      }
      ImGui::SameLine( 0.0f, 8.0f );
    }
  }
  else
  {
    RenderDisabled( ImGui::Button( m_currentPath.filename().string().c_str() ); )
  }

  ImGui::PopStyleColor();
  ImGui::EndGroup();
}

auto FileExplorerWindow::fileTexture( const File_& file ) -> VkDescriptorSet&
{
  auto extension = file.path.extension().string();
  if ( !m_spec.fileIcons.contains( extension ) )
    return m_spec.genericFileIcon;

  return m_spec.fileIcons.at( extension );
}

} // namespace gui
