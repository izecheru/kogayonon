#pragma once
#include "core/renderer/renderer.h"
#include "window/window.h"
#include "events/app_event.h"
#include "events/mouse_events.h"
#include "events/keyboard_events.h"

class App
{
public:
  App();
  void run();

  void onEvent(Event& event);

  bool onWindowResize(WindowResizeEvent& event);
  bool onWindowClose(WindowCloseEvent& event);
  bool onMouseMove(MouseMovedEvent& event);
  bool onMouseEnter(MouseEnteredEvent& event);
  bool onKeyPress(KeyPressedEvent& event);
  bool onScroll();

  static GLFWwindow* getWindow();

private:
  static inline float delta_time;
  static inline Window* m_window = nullptr;
  static inline Renderer* m_renderer = nullptr;
};
