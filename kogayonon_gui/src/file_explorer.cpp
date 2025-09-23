#include "gui/file_explorer.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/file_events.hpp"
#include "logger/logger.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/directory_watcher/directory_watcher.hpp"
using namespace kogayonon_logger;

namespace kogayonon_gui
{
FileExplorerWindow::FileExplorerWindow( std::string name, unsigned int folderTextureId, unsigned int fileTextureId )
    : ImGuiWindow( std::move( name ) )
    , m_currentPath( std::filesystem::current_path() / "resources" )
    , m_pDirWatcher( std::make_unique<kogayonon_utilities::DirectoryWatcher>( "resources\\" ) )
    , m_fileTextureId( fileTextureId )
    , m_folderTextureId( folderTextureId )
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
  std::filesystem::path p( path );
  auto ext = p.extension().string();
  return ext == ".jpg" || ext == ".png";
}

void FileExplorerWindow::onFileModified( kogayonon_core::FileModifiedEvent& e )
{
  Logger::info( "FILE MODIFIED ", e.getPath(), " name ", e.getName() );
}

void FileExplorerWindow::onFileCreated( kogayonon_core::FileCreatedEvent& e )
{
  Logger::info( "FILE CREATED ", e.getPath(), " name ", e.getName() );
}

void FileExplorerWindow::onFileDeleted( kogayonon_core::FileDeletedEvent& e )
{
  Logger::info( "FILE DELETED ", e.getPath(), " name ", e.getName() );
}

void FileExplorerWindow::onFileRenamed( kogayonon_core::FileRenamedEvent& e )
{
  Logger::info( "FILE RENAMED ", e.getPath(), " name ", e.getName() );
}

void FileExplorerWindow::draw()
{
  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    ImGui::End();
    return;
  }

  // I don't like default bg color
  ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 1, 0, 0, 0.5f ) );
  drawPathToolbar();
  ImGui::PopStyleColor( 1 );

  // for each imgui element we need an id that is unique, so I use this dirId to add it to already defined const char*
  int dirId = 0;
  for ( auto const& dirEntry : std::filesystem::directory_iterator( m_currentPath ) )
  {
    std::string id = "##file" + std::to_string( dirId++ );
    const auto& path = dirEntry.path();
    auto relativePath = std::filesystem::relative( path );
    if ( dirEntry.is_directory() )
    {
      ImGui::BeginGroup();
      if ( ImGui::ImageButton( id.c_str(), (ImTextureID)m_folderTextureId, ImVec2( 40, 40 ) ) )
      {
        m_currentPath = dirEntry.path();
      }
      ImGui::Text( "%s", dirEntry.path().filename().string().c_str() );
      ImGui::EndGroup();
    }
    else
    {
      // if it is a file
      ImGui::BeginGroup();
      ImGui::ImageButton( id.c_str(), (ImTextureID)m_fileTextureId, ImVec2( 40, 40 ) );
      if ( ImGui::BeginDragDropSource() )
      {
        std::string test = relativePath.string(); // "assets/textures/xyz.png" etc
        ImGui::SetDragDropPayload( "ASSET_DROP", test.c_str(), test.size(), ImGuiCond_Once );
        ImGui::EndDragDropSource();
      }
      ImGui::Text( "%s", dirEntry.path().filename().string().c_str() );
      ImGui::EndGroup();
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
  // if last element is resources than we don't render since we're at Assets root
  // if ( pathItems.at( pathItems.size() - 1 ) == "resources" )
  //{
  //  return;
  //}

  static int currentIndex = -1;

  ImGui::BeginGroup();
  ImVec2 pathSizeText = ImGui::CalcTextSize( "Current path" );
  ImGui::Text( "Current path", ImVec2( pathSizeText.x + 20.0f, 20.0f ) );
  for ( int i = 0; i < pathItems.size(); ++i )
  {
    ImGui::SameLine();
    // if we press on a folder from path toolbar
    ImVec2 textSize = ImGui::CalcTextSize( pathItems.at( i ).c_str() );
    if ( ImGui::Button( pathItems.at( i ).c_str(), ImVec2( textSize.x + 10.0f, 20.0f ) ) )
    {
      // don't set currentIndex if it is already clicked once
      if ( currentIndex != i )
      {
        // we get the index of the button we pressed
        currentIndex = i;
        break;
      }
    }
  }
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