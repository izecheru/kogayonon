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

gui::FileExplorerWindow::FileExplorerWindow( std::string const& name, FileExplorerSpec const& spec )
    : ImGuiWindow{ name }
    , m_currentDirectory{ nullptr }
    , m_pDirWatcher{ std::make_unique<DirectoryWatcher>( std::filesystem::absolute( "." ) ) }
    , m_pDispatcher{ std::make_unique<EventDispatcher>() }
    , m_hierarchy{ fs::current_path() / "engine_resources" }
    , m_spec{ spec }
    , m_searchStr{ "" }
{
    installHandlers();
    setCallback();
}

void gui::FileExplorerWindow::installHandlers()
{
    m_pDispatcher->addHandler<FileEvent, &FileExplorerWindow::onFileEvent>( *this );
}

void gui::FileExplorerWindow::setCallback()
{
    m_pDirWatcher->setCallback( [this]( std::string const& path, std::string const& name, FileEventType const& type ) {
        m_pDispatcher->dispatchEvent<FileEvent>( FileEvent{ path, name, type } );
    } );
}

bool gui::FileExplorerWindow::isTexture( std::string const& path )
{
    std::filesystem::path p{ path };
    auto ext = p.extension().string();
    return ext == ".jpg" || ext == ".png";
}

void gui::FileExplorerWindow::onFileEvent( FileEvent& e )
{
    core::EventDispatcher* eventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
    switch ( e.getType() )
    {
        // handle all the modify functionality
    case FileEventType::Modify: {
        if ( e.getPath().find( "colorConfig" ) != std::string::npos )
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

void gui::FileExplorerWindow::drawFileContextMenu( FileEntry const& file, std::string const& id )
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

auto gui::FileExplorerWindow::drawDirectoryHierarchy() -> void
{
    if ( !m_hierarchy.isInit() )
    {
        throw std::runtime_error( "directory hierarchy was not initialized prior" );
    }

    const float height = ImGui::GetContentRegionAvail().y;

    ImGui::BeginChild( "##dirExplorer", ImVec2{ 300.0f, height }, true );
    drawFromRoot( m_hierarchy.root() );
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild( "##filesPanel", ImVec2{ 0.0f, height }, true );
    drawFiles();
    ImGui::EndChild();
}

auto gui::FileExplorerWindow::drawNodes( DirectoryEntry& e ) -> void
{
    ImGui::PushID( e.path.string().c_str() );

    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
    for ( DirectoryEntry& child : e.children )
    {
        ImGui::SetNextItemOpen( child.open );

        const bool isLeaf = child.children.empty();

        std::string folderIcon = isLeaf ? ICON_MDI_FOLDER_OPEN : ICON_MDI_FOLDER;
        folderIcon += child.path.stem().string();

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoArrowDraw;

        if ( isLeaf )
        {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        const bool open = ImGui::TreeNodeEx( folderIcon.c_str(), flags );

        if ( !isLeaf && ImGui::IsItemToggledOpen() )
        {
            child.open = open;

            if ( !open )
            {
                m_currentDirectory = nullptr;
            }
        }

        const bool clicked = ImGui::IsItemClicked( ImGuiMouseButton_Left );
        const bool doubleClicked = ImGui::IsMouseDoubleClicked( ImGuiMouseButton_Left ) && ImGui::IsItemHovered();

        if ( !isLeaf )
        {
            if ( clicked && !open )
            {
                m_currentDirectory = &child;
            }
        }
        else
        {
            if ( doubleClicked )
            {
                m_currentDirectory = &child;
            }
        }

        if ( open && !isLeaf )
        {
            drawNodes( child );
            ImGui::TreePop();
        }
    }

    ImGui::PopID();
}

auto gui::FileExplorerWindow::drawFromRoot( DirectoryEntry& e ) -> void
{
    ImGui::SetNextItemOpen( e.open );

    std::string folderIcon( ICON_MDI_FOLDER );
    folderIcon += e.path.stem().string();

    const bool open =
        ImGui::TreeNodeEx( folderIcon.c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoArrowDraw );

    if ( ImGui::IsItemToggledOpen() )
    {
        e.open = open;
        // If this is uncommented, when a dir is closed then all subdirs are

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

void gui::FileExplorerWindow::render()
{
    if ( !begin() )
    {
        return;
    }

    drawDirectoryHierarchy();
    if ( m_hierarchy.needRebuild() )
    {
        // we previously set the node to the one who got its contents modified
        // and now we rebuild its children
        m_hierarchy.rebuildNode();
    }

    ImGui::End();
}

auto gui::FileExplorerWindow::fileTexture( FileEntry const& file ) -> VkDescriptorSet&
{
    std::string extension = file.path.extension().string();

    auto it = m_spec.fileIcons.find( extension );

    if ( it == m_spec.fileIcons.end() )
    {
        return m_spec.genericFileIcon;
    }

    return it->second;
}

auto gui::FileExplorerWindow::drawFiles() -> void
{
    if ( !m_currentDirectory )
        return;

    constexpr int kColumns = 10;

    if ( ImGui::BeginTable( "##fileTable", kColumns ) )
    {
        for ( size_t i = 0; i < m_currentDirectory->files.size(); ++i )
        {
            if ( i % kColumns == 0 )
            {
                ImGui::TableNextRow();
            }

            ImGui::TableNextColumn();

            FileEntry& f = m_currentDirectory->files[i];
            ImGui::Text( "%s", f.path.stem().string().c_str() );
        }
        ImGui::EndTable();
    }
}
