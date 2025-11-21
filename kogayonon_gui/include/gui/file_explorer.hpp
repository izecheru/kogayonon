#pragma once
#include <string_view>
#include "core/event/event.hpp"
#include "core/event/event_dispatcher.hpp"
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

namespace kogayonon_gui
{
class FileExplorerWindow : public ImGuiWindow
{
  struct File_
  {
    bool isDir{ false };
    std::string imguiId{ "##" };
    std::filesystem::path path;
  };

public:
  explicit FileExplorerWindow( std::string name, uint32_t folderTextureId, uint32_t fileTextureId );

  ~FileExplorerWindow() = default;

  void draw() override;

  // Event
  void onFileModified( kogayonon_core::FileModifiedEvent& e );
  void onFileCreated( kogayonon_core::FileCreatedEvent& e );
  void onFileRenamed( kogayonon_core::FileRenamedEvent& e );
  void onFileDeleted( kogayonon_core::FileDeletedEvent& e );

private:
  /**
   * @brief Draw the path, relative from resources folder, we can navigate back to "root" using it
   */
  void drawToolbar();

  /**
   * @brief Initializes a map of callbacks for the DirectoryWatcher to use and call when a file event is triggered
   */
  void installCommands();

  /**
   * @brief Adds event handlers and links them to onEvent functions from FilExplorerWindow
   */
  void installHandlers();

  bool isTexture( const std::string& path );

  /**
   * @brief Builds a vector of File_ instances for the current directory
   */
  void buildFileVector();

  /**
   * @brief Draws the context menu for files, here are defined funcs like Delete file and more to come
   * @param file The file we draw the context menu for
   * @param id This is the id for the ImGui::BeginPopupContextItem(id) since the filename is unique
   */
  void drawFileContextMenu( const File_& file, const std::string& id );

private:
  std::vector<File_> m_files;
  std::atomic_bool m_update;
  uint32_t m_folderTextureId;
  uint32_t m_fileTextureId;

  std::filesystem::path m_currentPath;
  std::unique_ptr<kogayonon_utilities::DirectoryWatcher> m_pDirWatcher;
  std::unique_ptr<kogayonon_core::EventDispatcher> m_pDispatcher;
};
} // namespace kogayonon_gui