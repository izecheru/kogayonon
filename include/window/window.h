#pragma once
#include <glad/glad.h>
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <unordered_set>
#include <functional>
#include <events/event.h>
#include <cstdint>
#include <core/key_codes.h>

namespace kogayonon
{
  using EventCallbackFn = std::function<void(Event&)>;

  struct window_props {
    const char* title;
    unsigned short width;
    unsigned short height;
    bool vsync;
    EventCallbackFn eventCallback;

    window_props(const char* t_title = "kogayonon",
      unsigned short t_width = 1800,
      unsigned short t_height = 900,
      bool t_vsync = true)
      : title(t_title), width(t_width), height(t_height), vsync(t_vsync) {
    }

    ~window_props() = default;
  };

  class Window {
  public:
    Window();
    virtual ~Window();

    void update();
    unsigned short getWidth() const;
    unsigned short getHeight() const;
    void setVsync();
    bool isVsync();
    void setViewport();
    void setEventCallbackFn(const EventCallbackFn& callback);
    GLFWwindow* getWindow();
    window_props& getWindowData();

  private:
    bool init(const window_props& props);

  private:
    GLFWwindow* m_window{};
    window_props m_data{};
  };
}
