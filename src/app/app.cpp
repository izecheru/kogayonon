#include <glm/glm.hpp>
#include <iostream>
#include <memory>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif  // !GLFW_INCLUDE_NONE

#include <filesystem>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "app/app.h"
#include "events/event_listener.h"
#include "core/input/input.h"
#include "core/logger.h"
#include "core/renderer/camera.h"
#include "core/model_loader/mesh.h"
#include "core/renderer/renderer.h"
#include "events/keyboard_events.h"
#include "events/mouse_events.h"
#include "window/window.h"
#include "core/model_loader/model.h"
#include "core/model_loader/model_loader.h"
#include "core/ui/imgui_interface.h"
#include "core/model_loader/model_manager.h"
#include "core/layer/layer_stack.h"
#include "core/layer/imgui_layer.h"

namespace kogayonon
{
  void checkPermissions(const std::filesystem::path& filePath)
  {
    try
    {
      auto perms = std::filesystem::status(filePath).permissions();

      std::cout << "Checking permissions for: " << filePath << std::endl;

      if ((perms & std::filesystem::perms::owner_read) != std::filesystem::perms::none)
        std::cout << "Owner has read permission.\n";
      else
        std::cout << "Owner does NOT have read permission.\n";

      if ((perms & std::filesystem::perms::owner_write) != std::filesystem::perms::none)
        std::cout << "Owner has write permission.\n";
      else
        std::cout << "Owner does NOT have write permission.\n";

      if ((perms & std::filesystem::perms::owner_exec) != std::filesystem::perms::none)
        std::cout << "Owner has execute permission.\n";
      else
        std::cout << "Owner does NOT have execute permission.\n";

    }
    catch (const std::filesystem::filesystem_error& e)
    {
      std::cerr << "Filesystem error: " << e.what() << std::endl;
    }
  }
  App::App()
  {
    m_window = std::make_unique<Window>();
    m_renderer = std::make_unique<Renderer>();

    // create layers here and push them
    ImguiLayer* imgui = new ImguiLayer(m_window->getWindow());
    m_renderer->pushLayer(imgui);

    EventListener::getInstance().addCallback<WindowResizeEvent>([this](Event& e) { return this->onWindowResize(static_cast<WindowResizeEvent&>(e)); });
    EventListener::getInstance().addCallback<WindowCloseEvent>([this](Event& e) { return this->onWindowClose(static_cast<WindowCloseEvent&>(e)); });
    EventListener::getInstance().addCallback<KeyPressedEvent>([this](Event& e) { return this->onKeyPress(static_cast<KeyPressedEvent&>(e)); });
  }

  void App::run()
  {
    const GLubyte* version = glGetString(GL_VERSION);
    Logger::logInfo("OpenGL Version: ", version);
    glEnable(GL_DEPTH_TEST);

    // all the events from the window are sent to the app.onEvent function and from there
    // to all the layers in the layer stack
    m_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });

    double prev_time = glfwGetTime();

    GLint maxVertices;
    glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxVertices);
    std::cout << "Max vertices per draw call: " << maxVertices << std::endl;

    m_renderer->pushShader("resources/shaders/3d_vertex.glsl", "resources/shaders/3d_fragment.glsl", "3d_shader");

    Camera& camera = Camera::getInstance();
    Shader& shader = m_renderer->getShader("3d_shader");
    ModelManager::getInstance().pushModel("resources/models/scene.gltf");

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
      glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glm::mat4 proj = glm::mat4(1.0f);
      float scaleFactor = 0.08f;
      glm::mat4 model_mat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
      glm::mat4 scale_mat = glm::scale(glm::mat4(6.0f), glm::vec3(10.08f));
      proj = glm::perspective(glm::radians(45.0f), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 200.0f);

      m_renderer->bindShader("3d_shader");

      shader.setMat4("model", model_mat);
      shader.setMat4("projection", proj);
      shader.setMat4("scaleMatrix", scale_mat);
      camera.cameraUniform(m_renderer->getShaderId("3d_shader"), "view");
      m_renderer->draw();
      m_renderer->unbindShader("3d_shader");

      ModelManager::getInstance().drawModels(m_renderer->getShader("3d_shader"));

      double current_time = glfwGetTime();
      delta_time = current_time - prev_time;
      camera.processKeyboard(delta_time);
      prev_time = current_time;
      m_window->update();

      if (glfwWindowShouldClose(m_window->getWindow()))
      {
        WindowCloseEvent close_event;

        // we dispatch the close event so if we need to do some cleanup we can now
        EventListener::getInstance().dispatch(close_event);
        break;
      }
    }
  }

  void App::onEvent(Event& event)
  {
    EventListener::getInstance().dispatch(event);
  }

  bool App::onMouseEnter(MouseEnteredEvent& event) { return true; }

  bool App::onWindowResize(WindowResizeEvent& event)
  {
    m_window->setViewport();
    return true;
  }

  bool App::onWindowClose(WindowCloseEvent& event)
  {
    Logger::logInfo("window close event");
    return true;
  }

  bool App::onMouseClicked(MouseClickedEvent& event)
  {
    return true;
  }

  bool App::onMouseMove(MouseMovedEvent& event)
  {
    Logger::logInfo("mouse move");
    return true;
  }

  bool App::onKeyPress(KeyPressedEvent& event)
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

  GLFWwindow* App::getWindow() { return m_window->getWindow(); }

  bool App::onScroll() { return true; }
}