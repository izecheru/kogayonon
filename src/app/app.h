#pragma once
#include "core/renderer/renderer.h"
#include "event/app_event.h"
#include "event/keyboard_events.h"
#include "event/mouse_events.h"
#include "window/window.h"

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

    void initializeContext();

  private:
    static inline bool capture_mouse = true;
    static inline double delta_time;
    static inline std::shared_ptr<Window> m_window;
  };
} // namespace kogayonon
