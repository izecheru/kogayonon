#pragma once
#include "event/app_event.h"
#include "input/keyboard_events.h"
#include "input/mouse_events.h"
#include "renderer/renderer.h"
#include "window/window.h"

namespace kogayonon
{
class App
{
public:
  App();
  ~App();
  void run() const;

  void initializeContext();

  // Events
private:
  bool onEvent(Event& e);
  bool onWindowResize(const WindowResizeEvent& e);
  bool onWindowClose(const WindowCloseEvent& e);

private:
  static inline double delta_time;
  static inline std::shared_ptr<Window> m_window;
  static inline bool m_running = true;
};
} // namespace kogayonon
