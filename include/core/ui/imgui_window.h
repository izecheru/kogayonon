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
    explicit ImguiWindow(std::string name) { m_props = new imgui_props(name); }
    virtual ~ImguiWindow() { delete m_props; }

    // Default implementations for all property functions
    virtual void setDocked(bool status) { m_props->m_docked = status; }
    virtual void setVisible(bool status) { m_props->m_visible = status; }
    virtual void setX(double x) { m_props->m_x = x; }
    virtual void setY(double y) { m_props->m_y = y; }

    virtual void draw() = 0;

  protected:
    struct imgui_props {
      std::string m_name;
      double m_x = 0.0;
      double m_y = 0.0;
      bool m_docked = false;
      bool m_can_move = true;
      bool m_visible = true;
      bool m_is_hovered = false;
      ImGuiWindowFlags m_flags;

      imgui_props(std::string t_name)
        : m_name(std::move(t_name)), m_x(0.0), m_y(0.0),
        m_docked(false), m_visible(true), m_is_hovered(false), m_can_move(true), m_flags(0) {
      }
    };

    imgui_props* m_props;
  };
}
