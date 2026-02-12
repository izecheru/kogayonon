#pragma once
#include <string_view>
#include "core/event/event.hpp"
#include "core/event/event_dispatcher.hpp"
#include "folder_display.hpp"
#include "imgui_window.hpp"
#include "utilities/directory_watcher/directory_watcher.hpp"

namespace kogayonon_core
{
class FileEvent;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class FileExplorerWindow : public ImGuiWindow
{
  struct File_
  {
    // is it or not a directory
    bool isDir{ false };

    // id used for the imgui
    std::string imguiId{ "##" };

    // path to the file
    std::filesystem::path path;

    // just a flag for search functionality, if the filename contains what we
    // search for then this is true, otherwise false (in case of searching)
    bool renderable{ true };
  };

public:
  explicit FileExplorerWindow( const std::string& name );

  ~FileExplorerWindow() = default;

  void draw() override;

  void onFileEvent( kogayonon_core::FileEvent& e );

private:
  /**
   * @brief Draw the path, relative from resources folder, we can navigate back to "root" using it
   */
  void drawToolbar();

  auto fileExtensionToTextureId( const std::string& fileExtension ) -> uint32_t;

  /**
   * @brief Initializes a map of callbacks for the DirectoryWatcher to use and call when a file event is triggered
   */
  void setCallback();

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

  void initIcons();

  /**
   * @brief Search for files that contain the string
   * @param toFind The string we search with and compare
   */
  void searchFor( const std::string& toFind );

private:
  std::vector<File_> m_files;
  std::atomic_bool m_update;

  std::unordered_map<std::string, uint32_t> m_icons;

  std::filesystem::path m_currentPath;
  std::unique_ptr<kogayonon_utilities::DirectoryWatcher> m_pDirWatcher;
  std::unique_ptr<kogayonon_core::EventDispatcher> m_pDispatcher;
};
} // namespace kogayonon_gui