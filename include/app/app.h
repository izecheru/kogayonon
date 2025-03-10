#pragma once
#include "core/renderer/renderer.h"
#include "window/window.h"
#include "events/app_event.h"
#include "events/mouse_events.h"
#include "events/keyboard_events.h"

namespace kogayonon
{
  class App
  {
  public:
    App();
    ~App();
    void run() const;

    void onEvent(Event& event) const;

    bool onWindowResize(const WindowResizeEvent& event) const;
    bool onWindowClose(const WindowCloseEvent& event) const;
    bool onMouseClicked(const MouseClickedEvent& event) const;
    bool onMouseMove(const MouseMovedEvent& event) const;
    bool onMouseEnter(const MouseEnteredEvent& event) const;
    bool onKeyPress(const KeyPressedEvent& event) const;
    bool onScroll() const;

    static GLFWwindow* getWindow();

  private:
    static inline bool capture_mouse = true;
    static inline double delta_time;
    static inline std::unique_ptr<Window> m_window;
    static inline std::unique_ptr<Renderer> m_renderer;
  };
}
