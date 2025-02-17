#pragma once

#include <string>
#include <imgui-1.91.8\imgui.h>

namespace kogayonon
{
  // TODO load the ini file and design the windows there
  // i need sliders and all that for a window so i dont know
  // of a way to load it programatically or write function with
  // params good enough to create any window i might need
  class ImguiWindow {
  public:
    ImguiWindow() = default;
    ~ImguiWindow() = default;

    ImguiWindow(std::string name) :m_props(std::move(name)) {}

    void setDocked(bool status);
    void setVisible(bool status);
    void setX(double x);
    void setY(double y);
    void draw();

  private:
    struct ImGuiProps {
      std::string m_name;
      double m_x = 0.0;
      double m_y = 0.0;
      bool m_docked = false;
      bool m_can_move = true;
      bool m_visible = true;
      bool m_is_hovered = false;
      ImGuiWindowFlags m_flags;

      ImGuiProps(std::string t_name)
        : m_name(std::move(t_name)), m_x(0.0), m_y(0.0),
        m_docked(false), m_visible(true), m_is_hovered(false), m_can_move(true), m_flags(ImGuiWindowFlags_NoCollapse) {
      }
    };

    ImGuiProps m_props;
  };
}
