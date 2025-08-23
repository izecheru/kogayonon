#include <glm/glm.hpp>
#include <iostream>
#include <memory>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#include <glad/glad.h>

#include "app/app.h"
#include "asset_manager/asset_manager.h"
#include "context_manager/context_manager.h"
#include "event/event_manager.h"
#include "klogger/klogger.h"
#include "renderer/camera.h"
#include "renderer/renderer.h"
#include "task/task_manager.h"
#include "ui/imgui_manager.h"
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
  }

  App::~App()
  {
    // LET THIS HERE IF YOU NEED LOGGING ON EXIT
    ContextManager::clear();
  }

  void App::run() const
  {
    const GLubyte* version = glGetString(GL_VERSION);
    ContextManager::klogger()->log(LogType::INFO, "OpenGL Version: ", version);
    ContextManager::klogger()->log(LogType::INFO, "Starting game engine");
    glEnable(GL_DEPTH_TEST);

    float prev_time = glfwGetTime();

    GLint maxVertices;
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxVertices);
    ContextManager::klogger()->log(LogType::INFO, "Max vertices per draw call: ", maxVertices);

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      ContextManager::renderer()->draw();
      m_window->update();
    }
  }

  void App::initializeContext()
  {
    ContextManager::addToContext(Context::KLoggerContext, std::make_shared<KLogger>("log.txt"));
    ContextManager::addToContext(Context::EventManagerContext, std::make_shared<EventManager>());
    ContextManager::addToContext(Context::AssetManagerContext, std::make_shared<AssetManager>());
    ContextManager::addToContext(Context::TaskManagerContext, std::make_shared<TaskManager>(10));
    ContextManager::addToContext(Context::CameraContext, std::make_shared<Camera>());
    ContextManager::addToContext(Context::RendererContext, std::make_shared<Renderer>(m_window->getWindow()));
  }

  bool App::onEvent(Event& e)
  {
    ContextManager::event_manager()->dispatch(e);
    return false;
  }

  bool App::onWindowResize(const WindowResizeEvent& e)
  {
    m_window->setViewport();
    ContextManager::klogger()->log(LogType::INFO, "App::onWindowResize(", e.getWidth(), " ", e.getHeight(), ")");
    return false;
  }
} // namespace kogayonon