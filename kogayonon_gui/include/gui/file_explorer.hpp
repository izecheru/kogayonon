#pragma once
#include <string_view>
#include "core/event/event.hpp"
#include "folder_display.hpp"
#include "imgui_window.hpp"
#include "utilities/directory_watcher/directory_watcher.hpp"

namespace kogayonon_core
{
class FileCreatedEvent;
class FileModifiedEvent;
class FileRenamedEvent;
class FileDeletedEvent;
} // namespace kogayonon_core

namespace kogayonon_utilities
{
class DirectoryWatcher;
}

namespace kogayonon_gui
{
class FileExplorerWindow : public ImGuiWindow
{
public:
  explicit FileExplorerWindow( std::string name, unsigned int folderTextureId, unsigned int fileTextureId );

  ~FileExplorerWindow() = default;

  void draw() override;

  void onFileModified( kogayonon_core::FileModifiedEvent& e );
  void onFileCreated( kogayonon_core::FileCreatedEvent& e );
  void onFileRenamed( kogayonon_core::FileRenamedEvent& e );
  void onFileDeleted( kogayonon_core::FileDeletedEvent& e );

private:
  void drawPathToolbar();
  /**
   * @brief Sets a set of callbacks for the DirectoryWatcher to call on a file event
   */
  void installCommands();

  /**
   * @brief Adds event handlers and ties them to onEvent from FilExplorerWindow class
   */
  void installHandlers();

private:
  bool isTexture( const std::string& path );

private:
  unsigned int m_folderTextureId = 0, m_fileTextureId = 0;
  std::filesystem::path m_currentPath;
  std::unique_ptr<kogayonon_utilities::DirectoryWatcher> m_pDirWatcher;
};
} // namespace kogayonon_gui