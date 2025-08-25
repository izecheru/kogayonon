#include "app/app.h"

#include <glm/glm.hpp>
#include <iostream>
#include <memory>

#include "asset_manager/asset_manager.h"
#include "context_manager/context_manager.h"
#include "event/event_manager.h"
#include "klogger/klogger.h"
#include "renderer/camera.h"
#include "renderer/renderer.h"
#include "task/task_manager.h"
#include "window/window.h"

namespace kogayonon
{
App::App()
{
  m_window = std::make_shared<Window>();
  m_window->setEventCallbackFn([this](Event& e) -> bool { return this->onEvent(e); });

  initializeContext();

  ContextManager::event_manager()->subscribe<WindowResizeEvent>(
      [this](const Event& e) -> bool { return this->onWindowResize((const WindowResizeEvent&)e); });

  ContextManager::event_manager()->subscribe<WindowCloseEvent>(
      [this](const Event& e) -> bool { return this->onWindowClose((const WindowCloseEvent&)e); });
}

App::~App()
{
  // LET THIS HERE IF YOU NEED LOGGING ON EXIT
  ContextManager::clear();
}

void App::run() const
{
  while (m_running)
  {
    ContextManager::renderer()->draw();
  }
}

void App::initializeContext()
{
  ContextManager::addToContext(Context::KLoggerContext, std::make_shared<KLogger>("log.txt"));
  ContextManager::addToContext(Context::EventManagerContext, std::make_shared<EventManager>());
  ContextManager::addToContext(Context::AssetManagerContext, std::make_shared<AssetManager>());
  ContextManager::addToContext(Context::TaskManagerContext, std::make_shared<TaskManager>(10));
  ContextManager::addToContext(Context::CameraContext, std::make_shared<Camera>());
  ContextManager::addToContext(Context::RendererContext, std::make_shared<Renderer>(m_window));
}

bool App::onEvent(Event& e)
{
  ContextManager::event_manager()->dispatch(e);
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