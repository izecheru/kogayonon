#pragma once
#include <memory>

namespace kogayonon_core {
class WindowResizeEvent;
} // namespace kogayonon_core

namespace kogayonon_window {
class Window;
} // namespace kogayonon_window

namespace kogayonon_app {
class App
{
  public:
    App();
    ~App();
    void pollEvents();
    void run();
    bool initialise();
    void rescaleMainViewport(int w, int h);
    bool onWindowResize(kogayonon_core::WindowResizeEvent& e);

  private:
    static inline std::shared_ptr<kogayonon_window::Window> m_pWindow;
    static inline bool m_running = true;
};
} // namespace kogayonon_app
