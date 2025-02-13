#pragma once

#include <string>

namespace kogayonon
{
  class ImguiWindow
  {
  public:
    ImguiWindow() = default;
    ~ImguiWindow() = default;

    ImguiWindow(std::string name, double x, double y, bool visible = true, bool docked = false) :m_name(name), m_x(x), m_y(y), m_docked(docked), m_visible(visible) {}

    void draw();

  private:
    std::string m_name;
    double m_x;
    double m_y;
    bool m_docked;
    bool m_visible;
  };
}
