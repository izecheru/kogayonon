#include "app/app.h"

#include <memory>

#include "klogger/klogger.h"
#include "registry_manager/registry_manager.h"
#include "window/window.h"

namespace kogayonon
{
App::App()
{
  KLogger::initialize("log.txt");

  m_window = std::make_shared<Window>();
  m_window->setEventCallbackFn([this](Event& e) -> bool { return this->onEvent(e); });

  REGISTRY().initialise(m_window);

  EVENT_MANAGER()->subscribe<WindowResizeEvent>(
      [this](const Event& e) -> bool { return this->onWindowResize((const WindowResizeEvent&)e); });

  EVENT_MANAGER()->subscribe<WindowCloseEvent>([this](const Event& e) -> bool { return this->onWindowClose((const WindowCloseEvent&)e); });
}

App::~App()
{
  KLogger::shutdown();
}

void App::run() const
{
  while (m_running)
  {
    RENDERER()->draw();
  }
}

bool App::onEvent(Event& e)
{
  EVENT_MANAGER()->dispatch(e);
  return false;
}

bool App::onWindowResize(const WindowResizeEvent& e)
{
  m_window->setViewport();
  return false;
}

bool App::onWindowClose(const WindowCloseEvent& e)
{
  m_running = false;
  return true;
}
} // namespace kogayonon