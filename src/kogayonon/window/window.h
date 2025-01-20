#pragma once
#include <string>
#include <glfw3.h>
#include <functional>
#include "../event/event.h"

namespace kogayonon {
  using EventCallbackFn = std::function<void(Event&)>;

  struct WindowProps {
    const char* Title;
    uint32_t Width;
    uint32_t Height;
    bool VSync;
    EventCallbackFn EventCallback;

    WindowProps(const char* title = "kogayonon",
                uint32_t width = 640,
                uint32_t height = 480,
                bool vsync = true)
      : Title(title), Width(width), Height(height), VSync(vsync) {}

    ~WindowProps() = default;
  };

  class Window {
  public:
    Window(const WindowProps& props);
    virtual ~Window();

    void onUpdate();
    void onClose();
    unsigned int getWidth() const;
    unsigned int getHeight() const;
    void setVsync();
    bool isVsync();
    void setEventCallbackFn(const EventCallbackFn& callback);
    GLFWwindow* getWindow();

  private:
    GLFWwindow* m_window;
    WindowProps m_data;
    bool setupWindow(const WindowProps& props);
  };

};
