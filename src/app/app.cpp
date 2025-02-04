#include <glm/glm.hpp>
#include <iostream>

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

App::App() {
  m_window = new Window();
  m_renderer = new Renderer();
}

App::~App() {
  delete m_window;
}

void App::run() {

  glEnable(GL_DEPTH_TEST);
  m_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });

  double prev_time = glfwGetTime();

  Model cat("models/cat/cat.obj");
  m_renderer->pushShader("shaders/3d_vertex.glsl", "shaders/3d_fragment.glsl", "3d_shader");
  Camera& camera = Camera::getInstance();
  while (!glfwWindowShouldClose(m_window->getWindow()))
  {
    m_renderer->bindShader("3d_shader");

    camera.processKeyboard(m_window->getWindow(), delta_time);
    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR)
    {
      std::cout << "OpenGL Error: " << err << " at line " << __LINE__ << std::endl;
    }
    glm::mat4 model = glm::mat4(1.0f);
    //glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);
    float scaleFactor = 2.40f; // Example scale factor
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, scaleFactor));
    proj = glm::perspective(glm::radians(45.0f), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 100.0f);

    int viewLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));

    int modelLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    int projLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

    int scaleLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "scaleMatrix");
    glUniformMatrix4fv(scaleLoc, 1, GL_FALSE, glm::value_ptr(scaleMatrix));

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // Dark gray background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    double current_time = glfwGetTime();
    delta_time = current_time - prev_time;
    prev_time = current_time;

    cat.draw(m_renderer->getShader("3d_shader"));

    m_window->update();
    m_renderer->unbindShader("3d_shader");
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
      return this->onMouseMove(e);
    });

  dispatcher.dispatch<KeyPressedEvent>([this](KeyPressedEvent& e) -> bool
    {
      if (e.getKeyCode() == KeyCode::Escape)
      {
        m_renderer->togglePolyMode();
      }
      // TODO must implement some switch to capture mouse or not
      if (e.getKeyCode() == KeyCode::M)
      {
      }
      if (e.getKeyCode() == KeyCode::V)
      {
        m_window->setVsync();
      }
      //if (e.getKeyCode() == KeyCode::F1)
      //{
      //  glfwSetWindowShouldClose(m_window->getWindow(), true);
      //}
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
