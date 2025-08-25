#pragma once

#include "imgui_window.h"

namespace kogayonon
{
class FileExplorerWindow : public ImGuiWindow
{
public:
  FileExplorerWindow(std::string&& name) : ImGuiWindow(std::move(name)) {}
};
} // namespace kogayonon
