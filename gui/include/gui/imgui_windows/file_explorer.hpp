#pragma once
#include <vulkan/vulkan.h>
#include "core/event/event.hpp"
#include "core/event/event_dispatcher.hpp"
#include "gui/imgui_windows/imgui_base.hpp"
#include "precompiled/pch.hpp"
#include "utilities/directory_watcher/directory_watcher.hpp"

namespace core
{
class FileEvent;
} // namespace core

struct FileExplorerSpec
{
  std::unordered_map<std::string, ImFont*>* fonts;
  // icons for the ui
  VkDescriptorSet iconGenericFolder;
  VkDescriptorSet genericFileIcon;
  std::unordered_map<std::string, VkDescriptorSet> fileIcons;
};

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

namespace gui
{

class FileExplorerWindow : public ImGuiWindow
{

public:
  explicit FileExplorerWindow( const std::string& name, const FileExplorerSpec& spec );

  ~FileExplorerWindow() = default;

  void render() override;

  void onFileEvent( core::FileEvent& e );

private:
  /**
   * @brief Draw the path, relative from resources folder, we can navigate back to "root" using it
   */
  void drawToolbar();

  auto fileTexture( const File_& file ) -> VkDescriptorSet&;

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

  /**
   * @brief Search for files that contain the string
   * @param toFind The string we search for
   */
  void searchFor( const std::string& toFind );

private:
  std::vector<File_> m_files;
  std::atomic_bool m_update;
  std::filesystem::path m_currentPath;
  std::unique_ptr<utilities::DirectoryWatcher> m_pDirWatcher;
  std::unique_ptr<core::EventDispatcher> m_pDispatcher;
  std::string m_searchStr;

  FileExplorerSpec m_spec;
};
} // namespace gui
