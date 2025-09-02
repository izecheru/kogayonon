#pragma once

#include <typeindex>

#include "display_manager.h"
#include "folder_display.h"
#include "imgui_window.h"

namespace kogayonon
{
class FileExplorerWindow : public ImGuiWindow
{
public:
  FileExplorerWindow(std::string name, std::string root_path) : ImGuiWindow(std::move(name)), m_root(std::move(root_path))
  {
    // TODO this should spawn the folders passed from root path so FolderDisplay should take  as param the path as well
    DisplayManager::addDisplay(m_displays, std::make_unique<FolderDisplay>("test", "/"));
  }

  ~FileExplorerWindow() override
  {
    m_displays.clear();
  }

  void draw() override;

private:
  std::string m_root;
  /**
   * @brief possibly using this to spawn children of a folder on the window
   */
  std::unordered_map<std::string, std::unique_ptr<Display>> m_displays;
};
} // namespace kogayonon
