#pragma once
#include <memory>

namespace kogayonon_core {
class WindowResizeEvent;
} // namespace kogayonon_core

namespace kogayonon_gui {
class SceneViewportWindow;
}

namespace kogayonon_window {
class Window;
} // namespace kogayonon_window

namespace kogayonon_app {
class App
{
  public:
    App() = default;
    ~App();
    void cleanup();
    void pollEvents();
    void run();
    bool initialise();
    bool initSDL();
    bool initRegistries();
    bool initGui();
    void rescaleMainViewport( int w, int h );
    bool onWindowResize( kogayonon_core::WindowResizeEvent& e );

    void callbackTest();

  private:
    std::shared_ptr<kogayonon_window::Window> m_pWindow;
    bool m_running = true;
};
} // namespace kogayonon_app
