#include <glfw3.h>
#include <iostream>
#include "app.h"
#include <window/window.h>
#include <event/keyboard/keyboard_events.h>
#include <core/logger/logger.h>

using std::cout;

namespace kogayonon
{
  App::App()
  {
    m_window = new Window();
  }

  void App::run()
  {
    // TODO: i must setup the render::init here and the imgui layer
    m_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });
    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
      m_window->onUpdate();
    }

    delete m_window;
  }

  void App::onEvent(Event& event)
  {
    Logger::logInfo(event.toString());
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e)->bool
      {
        Logger::logError("[dispatch updates on resize]\n");
        return this->onWindowResize(e);
      });

    dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent& e)->bool
      {
        Logger::logInfo("[dispatch updates on close]\n");
        return this->onWindowClose(e);
      });
  }

  bool App::onWindowResize(WindowResizeEvent& event)
  {
    glViewport(0, 0, event.getWidth(), event.getHeight());
    return true;
  }

  bool App::onWindowClose(WindowCloseEvent& event)
  {
    // TODO cleanup on window close
    return true;
  }
}  // namespace kogayonon
