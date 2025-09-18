#pragma once
#include <typeindex>
#include "folder_display.hpp"
#include "imgui_window.hpp"

namespace kogayonon_gui
{
class FileExplorerWindow : public ImGuiWindow
{
public:
  FileExplorerWindow( std::string name, std::string rootPath )
      : ImGuiWindow( name )
      , m_root( rootPath )
  {
    // add displays here
    auto folderDisplay = std::make_unique<FolderDisplay>( "Folder display", m_root );
    m_displays.emplace( "folder display", std::move( folderDisplay ) );
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
} // namespace kogayonon_gui