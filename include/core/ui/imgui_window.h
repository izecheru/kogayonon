#pragma once

#include <string>

namespace kogayonon
{
  // TODO load the ini file and design the windows there
  // i need sliders and all that for a window so i dont know
  // of a way to load it programatically or write function with
  // params good enough to create any window i might need
  class ImguiWindow
  {
  public:
    ImguiWindow() = default;
    ~ImguiWindow() = default;

    ImguiWindow(std::string name, double x, double y, bool visible = true, bool docked = false) :m_name(name), m_x(x), m_y(y), m_docked(docked), m_visible(visible) {}

    void draw();
    bool isHovered();

  private:
    std::string m_name;
    double m_x;
    double m_y;
    bool m_docked;
    bool m_visible;
    bool m_is_hovered;
  };
}
