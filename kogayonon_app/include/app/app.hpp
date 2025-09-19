#pragma once

#include <glad/glad.h>
#include <memory>

namespace kogayonon_core
{
class WindowResizeEvent;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class SceneViewportWindow;
} // namespace kogayonon_gui

namespace kogayonon_window
{
class Window;
} // namespace kogayonon_window

namespace kogayonon_rendering
{
class FrameBuffer;
} // namespace kogayonon_rendering

namespace kogayonon_app
{
class App
{
public:
  App() = default;
  ~App();
  void cleanup();
  void pollEvents();
  void run();

  bool init();
  bool initSDL();
  bool initRegistries();
  bool initGui();
  bool initScenes();

  void rescaleMainViewport( int w, int h );
  bool onWindowResize( kogayonon_core::WindowResizeEvent& e );

  static void glDebugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                               const GLchar* message, const void* userParam );

  void callbackTest();

private:
  std::shared_ptr<kogayonon_window::Window> m_pWindow;

  // untill I manage to find a place for this buffer, it'll stay here
  std::shared_ptr<kogayonon_rendering::FrameBuffer> m_pFrameBuffer;

  bool m_running = true;
};
} // namespace kogayonon_app
