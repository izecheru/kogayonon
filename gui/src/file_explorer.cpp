#include "gui/imgui_windows/file_explorer.hpp"
#include "core/event/config_event.hpp"
#include <imgui_impl_vulkan.h>
#include <imgui_stdlib.h>
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/file_events.hpp"
#include "gui/utils/font_keys.hpp"
#include "gui/utils/imgui_dragdrop_defines.hpp"
#include "gui/utils/imgui_utils.hpp"
#include "precompiled/pch.hpp"
#include "utilities/config_manager/config_manager.hpp"
#include "utilities/directory_watcher/directory_watcher.hpp"
#include "utilities/fonts/materialdesign.hpp"
#include "utilities/task_manager/task.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include "utilities/utils/utils.hpp"
#include "utilities/fonts/google_materialdesign.hpp"

using namespace core;
using namespace utilities;

namespace gui
{
FileExplorerWindow::FileExplorerWindow( const std::string& name, const FileExplorerSpec& spec )
    : ImGuiWindow{ name }
    , m_currentPath{ std::filesystem::absolute( "." ) / "engine_resources" }
    , m_pDirWatcher{ std::make_unique<DirectoryWatcher>( std::filesystem::absolute( "." ) ) }
    , m_pDispatcher{ std::make_unique<EventDispatcher>() }
    , m_hierarchy{ fs::current_path() / "engine_resources" }
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
  core::EventDispatcher* eventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();

  // handle all modify cases, currently we just do hot-load for shaders
  switch ( e.getType() )
  {
    // handle all the modify functionality
  case FileEventType::Modify: {
    if ( e.getPath().find( "glsl" ) != std::string::npos )
    {
    }
    else if ( e.getPath().find( "config" ) != std::string::npos )
    {
    }
    else if ( e.getPath().find( "colorConfig" ) != std::string::npos )
    {
      eventDispatcher->dispatchEvent( core::ConfigChangedEvent{} );
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
    KERROR( "something went wrong, enum FileEventType does not support this value" );
    break;
  }
}

void FileExplorerWindow::drawFileContextMenu( const FileEntry& file, const std::string& id )
{
  if ( ImGui::BeginPopupContextItem( id.c_str() ) )
  {
    if ( file.path.extension().string() == ".ttf" )
    {
      if ( ImGui::MenuItem( "Load font" ) )
      {
        auto assetManager = core::MainRegistry::getInstance().getAssetManager();
        auto taskManager = core::MainRegistry::getInstance().getTaskManager();

        auto callback = [assetManager, file]() { assetManager->loadFont( file.path.string() ); };
        auto task = taskManager->addTask( callback );
        taskManager->addTaskSetToPipe( task );
      }
    }
    if ( ImGui::MenuItem( "Delete file" ) )
    {
      std::filesystem::remove( file.path );
      KINFO( "removed {}", file.path.string() );
    }
    ImGui::EndPopup();
  }
}

auto FileExplorerWindow::drawDirectoryHierarchy() -> void
{
  if ( !m_hierarchy.isInit() )
  {
    throw std::runtime_error( "directory hierarchy was not initialized prior" );
  }
  else
  {
    drawFromRoot( m_hierarchy.root() );
  }
}

auto FileExplorerWindow::drawNodes( DirectoryEntry& e ) -> void
{
  ImGui::PushID( e.path.string().c_str() );

  ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
  for ( const auto& f : e.files )
  {
    ImGui::PushID( f.path.string().c_str() );

    if ( ImGui::TreeNodeEx( f.path.filename().string().c_str(),
                            ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen |
                              ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoArrowDraw ) )
    {
      if ( ImGui::BeginDragDropSource( ImGuiDragDropFlags_None ) )
      {
        std::string resultPath = f.path.string();
        ImGui::SetDragDropPayload( ASSET_DROP, resultPath.c_str(), resultPath.size() + 1, ImGuiCond_Once );
        ImGui::EndDragDropSource();
      }
      if ( ImGui::BeginPopupContextItem( "##fileMenu" ) )
      {
        if ( f.path.extension().string() == ".slang" )
        {
          if ( ImGui::BeginMenu( "Open with" ) )
          {
            // TODO actually check if we have those programs installed
            if ( ImGui::MenuItem( "Vs Code" ) )
            {
              ShellExecute( NULL, "open", "code", f.path.string().c_str(), NULL, SW_HIDE );
            }
            if ( ImGui::MenuItem( "Notepad" ) )
            {
              ShellExecute( NULL, "open", "notepad", f.path.string().c_str(), NULL, SW_HIDE );
            }
            if ( ImGui::MenuItem( "Notepad++" ) )
            {
              ShellExecute( NULL, "open", "notepad++", f.path.string().c_str(), NULL, SW_HIDE );
            }

            ImGui::EndMenu();
          }
        }
        if ( ImGui::MenuItem( "Delete file" ) )
        {
          std::filesystem::remove( f.path );
          m_hierarchy.setRebuild( true );
          m_hierarchy.setNode( &e );
        }
        ImGui::EndPopup();
      }
    }
    ImGui::PopID();
  }

  for ( auto& child : e.children )
  {
    ImGui::SetNextItemOpen( child.open );

    std::string folderIcon( ICON_MDI_FOLDER );
    folderIcon += child.path.stem().string();

    bool open =
      ImGui::TreeNodeEx( folderIcon.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoArrowDraw );

    if ( ImGui::IsItemToggledOpen() )
    {
      child.open = open;
    }

    if ( open )
    {
      drawNodes( child );
      ImGui::TreePop();
    }
  }

  ImGui::PopID();
}

auto FileExplorerWindow::drawFromRoot( DirectoryEntry& e ) -> void
{
  ImGui::SetNextItemOpen( e.open );

  std::string folderIcon( ICON_MDI_FOLDER );
  folderIcon += e.path.stem().string();

  const bool open =
    ImGui::TreeNodeEx( folderIcon.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoArrowDraw );

  if ( ImGui::IsItemToggledOpen() )
  {
    e.open = open;
    // if this is uncommented, when a dir is closed then all subdirs are
    // if ( !open )
    //{
    //   setOpenRecursive( e, false );
    // }
  }

  if ( open )
  {
    drawNodes( e );
    ImGui::TreePop();
  }
}

void FileExplorerWindow::render()
{
  if ( !begin() )
    return;

  drawDirectoryHierarchy();
  if ( m_hierarchy.needRebuild() )
  {
    // we previously set the node to the one who got its contents modified
    m_hierarchy.rebuildNode();
  }

  ImGui::End();
}

auto FileExplorerWindow::fileTexture( const FileEntry& file ) -> VkDescriptorSet&
{
  auto extension = file.path.extension().string();
  if ( !m_spec.fileIcons.contains( extension ) )
    return m_spec.genericFileIcon;

  return m_spec.fileIcons.at( extension );
}

} // namespace gui
