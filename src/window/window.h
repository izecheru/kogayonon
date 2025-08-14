#pragma once
#include <glad/glad.h>
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <GLFW/glfw3.h>
#include <core/key_codes.h>
#include <event/event.h>

#include <cstdint>
#include <functional>
#include <unordered_set>

namespace kogayonon
{
  using EventCallbackFn = std::function<void(Event&)>;

  struct window_props
  {
    const char* title;
    unsigned short width;
    unsigned short height;
    bool vsync;
    EventCallbackFn eventCallback;

    explicit window_props(const char* t_title = "kogayonon", unsigned short t_width = 900, unsigned short t_height = 500,
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
    void setEventCallbackFn(const EventCallbackFn& callback);
    GLFWwindow* getWindow();
    window_props& getWindowData();

  private:
    bool init(const window_props& props);

  private:
    GLFWwindow* m_window{};
    window_props m_data{};
  };
} // namespace kogayonon
