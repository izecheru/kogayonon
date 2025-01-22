#pragma once
#include "window/window.h"
#include "events/app_event.h"
#include "renderer/renderer.h"

namespace kogayonon
{
  class App {
  public:
    App();

    /// <summary>
    /// The main loop is here at the moment
    /// </summary>
    void run();

    /// <summary>
    /// Function that receives all the events from the app
    /// </summary>
    /// <param name="event"></param>
    void onEvent(Event& event);

    /// <summary>
    /// Do updates on window resize
    /// </summary>
    /// <param name="event"></param>
    /// <returns></returns>
    bool onWindowResize(WindowResizeEvent& event);

    /// <summary>
    /// Do cleanup on window close
    /// </summary>
    /// <param name="event"></param>
    /// <returns></returns>
    bool onWindowClose(WindowCloseEvent& event);

  private:
    Window* m_window;
    Renderer* m_renderer;
  };
}
