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
  explicit ImGuiWindow(std::string name)
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
    m_props->x = x;
  }

  virtual void setY(double y)
  {
    m_props->y = y;
  }

  int width()
  {
    return m_props->width;
  }

  int height()
  {
    return m_props->height;
  }

  virtual void draw() = 0;

  virtual void setCallback(const RenderCallbackFn& func)
  {
    m_render_callback = func;
  }

protected:

  // if we ever need to pass a rendering func in the middle of the window or something
  // i made this for the scene viewport so i can pass functions from Renderer class
  RenderCallbackFn m_render_callback;

  struct imgui_props
  {
    std::string m_name;
    double x = 0.0;
    double y = 0.0;
    int width = 0;
    int height = 0;
    bool is_docked = false;
    bool can_move = true;
    bool visible = true;
    bool is_hovered = false;
    bool can_resize = false;
    ImGuiWindowFlags flags;

    explicit imgui_props(std::string t_name)
        : m_name(std::move(t_name)), x(0), y(0), is_docked(false), visible(true), is_hovered(false), can_move(true), can_resize(false),
          flags(0)
    {}
  };

  std::unique_ptr<imgui_props> m_props = nullptr;
};
} // namespace kogayonon
