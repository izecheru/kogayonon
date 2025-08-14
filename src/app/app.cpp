#include <glm/glm.hpp>
#include <iostream>
#include <memory>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif // !GLFW_INCLUDE_NONE

#include <glad/glad.h>

#include <filesystem>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "app/app.h"
#include "core/asset_manager/asset_manager.h"
#include "core/context_manager/context_manager.h"
#include "core/input/input.h"
#include "core/klogger/klogger.h"
#include "core/layer/imgui_layer.h"
#include "core/layer/layer_stack.h"
#include "core/layer/world_layer.h"
#include "core/renderer/camera.h"
#include "core/renderer/renderer.h"
#include "core/task/task_manager.h"
#include "core/time_tracker/time_tracker.h"
#include "core/ui/imgui_interface.h"
#include "event/event_listener.h"
#include "event/keyboard_events.h"
#include "event/mouse_events.h"
#include "window/window.h"

namespace kogayonon
{
  App::App()
  {
    ContextManager::addToContext(Context::KLoggerContext, std::make_shared<KLogger>("log.txt"));
    ContextManager::addToContext(Context::AssetManagerContext, std::make_shared<AssetManager>());
    ContextManager::addToContext(Context::TaskManagerContext, std::make_shared<TaskManager>(10));

    m_window   = std::make_unique<Window>();
    m_renderer = std::make_unique<Renderer>();

    // TODO this should be done in the renderer not here
    m_renderer->pushShader("resources/shaders/3d_vertex.glsl", "resources/shaders/3d_fragment.glsl", "3d_shader");

    // the order you push layers here is the order of rendering, so imgiui would be drawn on
    // everything below it
    m_renderer->pushLayer(std::make_unique<WorldLayer>(m_renderer->getShader("3d_shader"))); // 0
    m_renderer->pushLayer(std::make_unique<ImguiLayer>(m_window->getWindow()));              // 1

    EventListener::getInstance()->addCallback<WindowResizeEvent>(
        [this](const Event& e) { return this->onWindowResize((const WindowResizeEvent&)(e)); });
    EventListener::getInstance()->addCallback<WindowCloseEvent>(
        [this](const Event& e) { return this->onWindowClose((const WindowCloseEvent&)(e)); });
    EventListener::getInstance()->addCallback<KeyPressedEvent>(
        [this](const Event& e) { return this->onKeyPress((const KeyPressedEvent&)(e)); });
  }

  App::~App()
  {
    m_window.reset();
    m_renderer.reset();

    // LET THIS HERE IF YOU NEED LOGGING ON EXIT
    ContextManager::clear();
  }

  void App::run() const
  {
    const GLubyte* version = glGetString(GL_VERSION);
    ContextManager::klogger()->log(LogType::INFO, "OpenGL Version: ", version);
    ContextManager::klogger()->log(LogType::INFO, "Starting game engine");
    glEnable(GL_DEPTH_TEST);

    // all the events from the window are sent to the app.onEvent function and
    // from there to all the layers in the layer stack
    m_window->setEventCallbackFn([this](Event& e) { this->onEvent(e); });

    // should probably handle this with Timer class or something
    float prev_time = glfwGetTime();

    GLint maxVertices;
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxVertices);
    ContextManager::klogger()->log(LogType::INFO, "Max vertices per draw call: ", maxVertices);
    Camera* camera = Camera::getInstance();
    Shader& shader = m_renderer->getShader("3d_shader");

    Model my_model("resources/models/sphere.gltf");

    // Model my_model2("resources/models/serialized_models/spehere.bin");

    // Model my_model3("resources/models/cube.gltf");

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
      glClearColor(0.3f, 0.0f, 1.0f, 0.3f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      auto proj           = glm::mat4(1.0f);
      float scale_factor  = 0.08f;
      glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -200.0f));
      glm::mat4 scale_mat = glm::scale(glm::mat4(6.0f), glm::vec3(10.08f * scale_factor));
      proj = glm::perspective(glm::radians(45.0f), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 4000.0f);

      m_renderer->bindShader("3d_shader");
      shader.setMat4("model", model_mat);
      shader.setMat4("projection", proj);
      shader.setMat4("scaleMatrix", scale_mat);
      camera->cameraUniform(m_renderer->getShaderId("3d_shader"), "view");

      m_renderer->draw();
      m_renderer->unbindShader("3d_shader");

      float current_time = glfwGetTime();
      TimeTracker::getInstance()->setDelta(current_time - prev_time);
      camera->processKeyboard();
      prev_time = current_time;
      m_window->update();

      if (glfwWindowShouldClose(m_window->getWindow()))
      {
        WindowCloseEvent close_event;

        // we dispatch the close event so if we need to do some cleanup we can now
        EventListener::getInstance()->dispatch(close_event);
        break;
      }
    }
  }

  void App::onEvent(Event& event) const
  {
    EventListener::getInstance()->dispatch(event);
  }

  bool App::onMouseEnter(const MouseEnteredEvent& event) const
  {
    return true;
  }

  bool App::onWindowResize(const WindowResizeEvent& event) const
  {
    m_window->setViewport();
    return true;
  }

  bool App::onWindowClose(const WindowCloseEvent& event) const
  {
    ContextManager::klogger()->log(LogType::INFO, "window close event");
    return true;
  }

  bool App::onMouseClicked(const MouseClickedEvent& event) const
  {
    return true;
  }

  bool App::onMouseMove(const MouseMovedEvent& event) const
  {
    ContextManager::klogger()->log(LogType::INFO, "mouse move");
    return true;
  }

  bool App::onKeyPress(const KeyPressedEvent& event) const
  {
    switch (event.getKeyCode())
    {
    case KeyCode::F1:
      m_renderer->togglePolyMode();
      break;
    case KeyCode::Escape:
      glfwSetWindowShouldClose(m_window->getWindow(), true);
      break;
    case KeyCode::C:
      glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
      break;
    default:
      return false;
    }
    return true;
  }

  GLFWwindow* App::getWindow()
  {
    return m_window->getWindow();
  }

  bool App::onScroll() const
  {
    return true;
  }
} // namespace kogayonon