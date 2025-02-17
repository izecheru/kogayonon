#include <glm/glm.hpp>
#include <iostream>
#include <memory>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif  // !GLFW_INCLUDE_NONE

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "app/app.h"
#include "events/event_listener.h"
#include "core/input/input.h"
#include "core/logger.h"
#include "core/renderer/camera.h"
#include "core/renderer/mesh.h"
#include "core/renderer/renderer.h"
#include "events/keyboard_events.h"
#include "events/mouse_events.h"
#include "window/window.h"
#include "core/renderer/model.h"
#include "core/model_loader/model_loader.h"
#include "core/ui/imgui_interface.h"

#include "core/layer/layer_stack.h"
#include "core/layer/imgui_layer.h"

namespace kogayonon
{
  App::App() {
    m_window = std::make_unique<Window>();
    m_renderer = std::make_unique<Renderer>();

    // create layers here and push them
    ImguiLayer* imgui = new ImguiLayer(m_window->getWindow());
    m_renderer->pushLayer(imgui);

    EventListener::getInstance().addCallback<WindowResizeEvent>([this](Event& e) { return this->onWindowResize(static_cast<WindowResizeEvent&>(e)); });
    EventListener::getInstance().addCallback<WindowCloseEvent>([this](Event& e) { return this->onWindowClose(static_cast<WindowCloseEvent&>(e)); });
    EventListener::getInstance().addCallback<KeyPressedEvent>([this](Event& e) { return this->onKeyPress(static_cast<KeyPressedEvent&>(e)); });
  }

  void App::run() {
    glEnable(GL_DEPTH_TEST);

    // all the events from the window are sent to the app.onEvent function and from there
    // to all the layers in the layer stack
    m_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });

    double prev_time = glfwGetTime();

    m_renderer->pushShader("shaders/3d_vertex.glsl", "shaders/3d_fragment.glsl", "3d_shader");
    Model my_model(std::string("models/stairs/scene.gltf"), m_renderer->getShader("3d_shader"));

    Camera& camera = Camera::getInstance();

    Shader& shader = m_renderer->getShader("3d_shader");

    while (!glfwWindowShouldClose(m_window->getWindow())) {
      glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 proj = glm::mat4(1.0f);
      float scaleFactor = 0.08f;
      glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, scaleFactor));
      proj = glm::perspective(glm::radians(45.0f), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 200.0f);

      m_renderer->bindShader("3d_shader");
      shader.setMat4("model", model);
      shader.setMat4("projection", proj);
      shader.setMat4("scaleMatrix", scaleMatrix);
      camera.cameraUniform(m_renderer->getShaderId("3d_shader"), "view");

      my_model.draw(m_renderer->getShader("3d_shader"));
      m_renderer->draw();

      m_renderer->unbindShader("3d_shader");
      double current_time = glfwGetTime();
      delta_time = current_time - prev_time;
      camera.processKeyboard(delta_time);
      prev_time = current_time;
      m_window->update();

      if (glfwWindowShouldClose(m_window->getWindow())) {
        WindowCloseEvent close_event;

        // we publish the close event so if we need to do some cleanup we can now
        EventListener::getInstance().dispatch(close_event);
        break;
      }
    }
  }

  void App::onEvent(Event& event) {
    EventListener::getInstance().dispatch(event);
  }

  bool App::onMouseEnter(MouseEnteredEvent& event) { return true; }

  //TODO things look stretched if I resize because i don't update the projection matrix i think
  bool App::onWindowResize(WindowResizeEvent& event) {
    m_window->setViewport();
    return true;
  }

  bool App::onWindowClose(WindowCloseEvent& event) {
    Logger::logInfo("window close event");
    return true;
  }

  bool App::onMouseClicked(MouseClickedEvent& event) {
    return true;
  }

  bool App::onMouseMove(MouseMovedEvent& event) {
    Logger::logInfo("mouse move");
    return true;
  }

  bool App::onKeyPress(KeyPressedEvent& event) {
    switch (event.getKeyCode()) {
      case KeyCode::F1:
        m_renderer->togglePolyMode();
        break;
      case KeyCode::Escape:
        glfwSetWindowShouldClose(m_window->getWindow(), true);
        break;
      default:
        return false;
    }
    return true;
  }

  GLFWwindow* App::getWindow() { return m_window->getWindow(); }

  bool App::onScroll() { return true; }
}