#pragma once
#include <vulkan/vulkan.h>
#include "core/event/event.hpp"
#include "core/event/event_dispatcher.hpp"
#include "gui/imgui_windows/imgui_base.hpp"
#include "precompiled/pch.hpp"
#include "utilities/directory_watcher/directory_watcher.hpp"
#include "gui/directory_hierarchy.hpp"

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

namespace gui
{

class FileExplorerWindow : public ImGuiWindow
{

public:
  explicit FileExplorerWindow( const std::string& name, const FileExplorerSpec& spec );

  ~FileExplorerWindow() = default;

  auto drawFromRoot( DirectoryEntry& e ) -> void;

  auto render() -> void override;

  auto onFileEvent( core::FileEvent& e ) -> void;

private:
  auto fileTexture( const FileEntry& file ) -> VkDescriptorSet&;

  /**
   * @brief Initializes a map of callbacks for the DirectoryWatcher to use and call when a file event is triggered
   */
  auto setCallback() -> void;

  /**
   * @brief Adds event handlers and links them to onEvent functions from FilExplorerWindow
   */
  auto installHandlers() -> void;

  auto isTexture( const std::string& path ) -> bool;

  /**
   * @brief Draws the context menu for files, here are defined funcs like Delete file and more to come
   * @param file The file we draw the context menu for
   * @param id This is the id for the ImGui::BeginPopupContextItem(id) since the filename is unique
   */
  auto drawFileContextMenu( const FileEntry& file, const std::string& id ) -> void;

  auto drawDirectoryHierarchy() -> void;
  auto drawNodes( DirectoryEntry& e ) -> void;

private:
  DirectoryHierarchy m_hierarchy;
  std::filesystem::path m_currentPath;
  std::unique_ptr<utilities::DirectoryWatcher> m_pDirWatcher;
  std::unique_ptr<core::EventDispatcher> m_pDispatcher;
  std::string m_searchStr;
  bool init;

  FileExplorerSpec m_spec;
};
} // namespace gui
