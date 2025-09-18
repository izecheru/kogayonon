#pragma once
#include "gui/imgui_window.hpp"

namespace kogayonon_gui
{
class PerformanceWindow : public ImGuiWindow
{
public:
  explicit PerformanceWindow( std::string name );
  ~PerformanceWindow() = default;

  void draw() override;

private:
};
} // namespace kogayonon_gui