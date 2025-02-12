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
#include "core/logger.h"
#include "core/renderer/camera.h"
#include "core/renderer/mesh.h"
#include "core/renderer/renderer.h"
#include "events/keyboard_events.h"
#include "events/mouse_events.h"
#include "window/window.h"
#include "core/renderer/model.h"
#include "core/model_loader/model_loader.h"

namespace kogayonon
{
  App::App() {
    m_window = std::make_unique<Window>();
    m_renderer = std::make_unique<Renderer>();
  }

  void App::run() {
    glEnable(GL_DEPTH_TEST);
    m_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });

    double prev_time = glfwGetTime();

    m_renderer->pushShader("shaders/3d_vertex.glsl", "shaders/3d_fragment.glsl", "3d_shader");
    std::string my_model_path = "models/untitled.obj";
    Model my_model(my_model_path, m_renderer->getShader("3d_shader"));
    Camera& camera = Camera::getInstance();
    Shader& shader = m_renderer->getShader("3d_shader");
    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
      glClearColor(0.95f, 0.95f, 0.95f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      camera.processKeyboard(m_window->getWindow(), delta_time);
      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 proj = glm::mat4(1.0f);
      float scaleFactor = 1.0f;
      glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, scaleFactor));
      proj = glm::perspective(glm::radians(45.0f), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 200.0f);

      m_renderer->bindShader("3d_shader");
      shader.setMat4("model", model);
      shader.setMat4("projection", proj);
      shader.setMat4("scaleMatrix", scaleMatrix);
      camera.cameraUniform(m_renderer->getShaderId("3d_shader"), "view");
      my_model.render(m_renderer->getShader("3d_shader"));
      m_renderer->unbindShader("3d_shader");

      double current_time = glfwGetTime();
      delta_time = current_time - prev_time;
      prev_time = current_time;
      m_window->update();
    }
  }

  void App::onEvent(Event& event) {
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) -> bool
      {
        Logger::logError("[dispatch updates on resize]\n");
        return this->onWindowResize(e);
      });

    dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) -> bool
      {
        Logger::logError("[dispatch updates on close]\n");
        return this->onWindowClose(e);
      });

    dispatcher.dispatch<MouseMovedEvent>([this](MouseMovedEvent& e) -> bool
      {
        Camera& camera = Camera::getInstance();
        camera.processMouseMoved(e.getX(), e.getY());
        camera.cameraUniform(m_renderer->getShaderId("3d_shader"), "view");
        return this->onMouseMove(e);
      });

    dispatcher.dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) -> bool
      {
        if (e.getKeyCode() == KeyCode::Escape)
        {
          glfwSetWindowShouldClose(m_window->getWindow(), true);
        }
        // TODO must implement some switch to capture mouse or not
        if (e.getKeyCode() == KeyCode::M)
        {
        }
        if (e.getKeyCode() == KeyCode::V)
        {
          m_window->setVsync();
        }
        if (e.getKeyCode() == KeyCode::F1)
        {
          m_renderer->togglePolyMode();
        }
        return this->onKeyPress(e);
      });

    dispatcher.dispatch<MouseEnteredEvent>([this](MouseEnteredEvent& e) -> bool
      {
        if (e.hasEntered())
        {
          glfwSetInputMode(m_window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
          glfwSetInputMode(m_window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        return this->onMouseEnter(e);
      });
  }

  bool App::onMouseEnter(MouseEnteredEvent& event) { return true; }

  bool App::onWindowResize(WindowResizeEvent& event) {
    m_window->setViewport(event.getWidth(), event.getHeight());
    return true;
  }

  bool App::onWindowClose(WindowCloseEvent& event) { return true; }

  bool App::onMouseMove(MouseMovedEvent& event) { return true; }

  bool App::onKeyPress(KeyPressedEvent& event) { return true; }

  GLFWwindow* App::getWindow() { return m_window->getWindow(); }

  bool App::onScroll() { return true; }
}
