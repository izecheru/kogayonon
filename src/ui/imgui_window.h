#pragma once

#include <imgui.h>

#include <memory>
#include <string>

namespace kogayonon
{
class ImGuiWindow
{
public:
  explicit ImGuiWindow(std::string&& name)
  {
    m_props = std::make_unique<imgui_props>(std::move(name));
  }

  virtual ~ImGuiWindow() {}

  // Default implementations for all property functions
  virtual void setDocked(bool status)
  {
    m_props->is_docked = status;
  }

  virtual void setVisible(bool status)
  {
    m_props->visible = status;
  }

  virtual void setX(double x)
  {
    m_props->m_x = x;
  }

  virtual void setY(double y)
  {
    m_props->m_y = y;
  }

  virtual void draw() = 0;

protected:
  struct imgui_props
  {
    std::string m_name;
    double m_x = 0.0;
    double m_y = 0.0;
    bool is_docked = false;
    bool can_move = true;
    bool visible = true;
    bool is_hovered = false;
    bool can_resize = false;
    ImGuiWindowFlags m_flags;

    explicit imgui_props(std::string&& t_name)
        : m_name(std::move(t_name)), m_x(0.0), m_y(0.0), is_docked(false), visible(true), is_hovered(false), can_move(true),
          can_resize(false), m_flags(0)
    {}
  };

  std::unique_ptr<imgui_props> m_props = nullptr;
};
} // namespace kogayonon
