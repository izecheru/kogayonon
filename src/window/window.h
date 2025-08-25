#pragma once
#include <SDL.h>
#include <event/event.h>

#include <functional>

namespace kogayonon
{
using EventCallbackFn = std::function<bool(Event&)>;

struct window_props
{
  const char* title;
  unsigned short width;
  unsigned short height;
  bool vsync;
  EventCallbackFn callback;

  explicit window_props(const char* t_title = "kogayonon engine", unsigned short t_width = 1900, unsigned short t_height = 1000,
                        bool t_vsync = true)
      : title(t_title), width(t_width), height(t_height), vsync(t_vsync)
  {}

  ~window_props() = default;
};

class Window
{
public:
  Window();
  ~Window();

  void update();
  unsigned short getWidth() const;
  unsigned short getHeight() const;
  void setVsync();
  bool isVsync();
  void setViewport();
  void maximize();
  window_props& getWindowData();
  void setEventCallbackFn(const EventCallbackFn& callback);

  SDL_Window* getWindow();

  inline SDL_GLContext getContext() noexcept
  {
    return m_gl_context;
  }

private:
  bool init(const window_props& props);

private:
  SDL_Window* m_window{};
  window_props m_data{};
  bool m_initialized = false;
  SDL_GLContext m_gl_context = nullptr;
};
} // namespace kogayonon
