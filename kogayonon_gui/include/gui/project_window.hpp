#pragma once
#include "gui/imgui_window.hpp"

namespace kogayonon_gui
{
class ProjectWindow : public ImGuiWindow
{
public:
  explicit ProjectWindow( const std::string& name );

  void draw() override;

private:
};
} // namespace kogayonon_gui