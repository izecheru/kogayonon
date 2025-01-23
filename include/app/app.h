#pragma once
#include "renderer/renderer.h"
#include "window/window.h"
#include "events/app_event.h"

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

    static GLFWwindow* getWindow();
    static void scrollCallback(GLFWwindow* window, double x, double y);

  private:
    static inline Window* m_window = nullptr;
    static inline Renderer* m_renderer = nullptr;
  };
}
