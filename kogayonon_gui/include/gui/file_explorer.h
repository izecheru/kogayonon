#pragma once
#include <typeindex>
#include "folder_display.h"
#include "imgui_window.h"

namespace kogayonon_gui {

class FileExplorerWindow : public ImGuiWindow
{
  public:
    FileExplorerWindow(std::string name, std::string root_path) : ImGuiWindow(std::move(name)), m_root(std::move(root_path))
    {
        // add displays here
        auto folder_display = std::make_unique<FolderDisplay>("Folder display", m_root);
        m_displays.emplace("folder display", std::move(folder_display));
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
