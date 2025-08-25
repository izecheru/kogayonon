#pragma once
#include <imgui.h>

#include <functional>
#include <memory>
#include <string>

namespace kogayonon
{
class ImGuiWindow
{
  using RenderCallbackFn = std::function<void()>;

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

  double width()
  {
    return m_props->m_width;
  }

  double height()
  {
    return m_props->m_height;
  }

  virtual void draw() = 0;

  virtual void setCallback(const RenderCallbackFn& func)
  {
    m_render_callback = func;
  }

protected:
  RenderCallbackFn m_render_callback;

  struct imgui_props
  {
    std::string m_name;
    double m_x = 0.0;
    double m_y = 0.0;
    double m_width = 0.0;
    double m_height = 0.0;
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
