#pragma once
#include <glfw/glfw3.h>
#include "events/event.h"
#include <functional>

namespace kogayonon
{
  using EventCallbackFn = std::function<void(Event&)>;

  struct WindowProps
  {
    const char* m_title;
    uint32_t m_width;
    uint32_t m_height;
    bool m_vsync;
    EventCallbackFn eventCallback;

    WindowProps(const char* title = "kogayonon",
      uint32_t width = 640,
      uint32_t height = 480,
      bool vsync = true)
      : m_title(title), m_width(width), m_height(height), m_vsync(vsync) {}

    ~WindowProps() = default;
  };

  class Window
  {
  public:
    Window();
    virtual ~Window();

    void update();
    void onClose();
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    void setVsync(bool enabled);
    bool isVsync();
    void setViewport(int width, int height);
    void setViewport();
    void setEventCallbackFn(const EventCallbackFn& callback);
    GLFWwindow* getWindow();
    WindowProps getWindowData();

  private:
    GLFWwindow* m_window;
    WindowProps m_data;
    bool init(const WindowProps& props);
  };

};
