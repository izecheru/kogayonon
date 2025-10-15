#pragma once

#include <glad/glad.h>
#include <memory>

namespace kogayonon_core
{
class WindowResizeEvent;
class WindowCloseEvent;
class ProjectLoadEvent;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class SceneViewportWindow;
} // namespace kogayonon_gui

namespace kogayonon_window
{
class Window;
} // namespace kogayonon_window

namespace kogayonon_app
{
class App
{
public:
  App();
  ~App();
  void cleanup() const;
  void pollEvents();
  void run();

  bool init();
  bool initSDL();
  bool initRegistries() const;
  bool initGui();
  bool initScenes() const;

  void rescaleMainViewport( int w, int h );
  bool onWindowResize( const kogayonon_core::WindowResizeEvent& e );
  void onWindowClose( const kogayonon_core::WindowCloseEvent& e );
  void onProjectLoad( const kogayonon_core::ProjectLoadEvent& e );

  static void glDebugCallback( GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                               const GLchar* message, const void* userParam );

private:
  std::shared_ptr<kogayonon_window::Window> m_pWindow;

  bool m_running = true;
};
} // namespace kogayonon_app
