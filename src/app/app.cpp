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

  ModelLoader loader;
  loader.importAsset("models/cat/cat.glb");
  while (!glfwWindowShouldClose(m_window->getWindow()))
  {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // Dark gray background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float current_time = glfwGetTime();
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
      //Camera& camera = Camera::getInstance();
      //camera.processMouseMoved(e.getX(), e.getY());
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
      if (e.getKeyCode() == KeyCode::F1)
      {
        glfwSetWindowShouldClose(m_window->getWindow(), true);
      }
      return this->onKeyPress(e);
    });

  dispatcher.dispatch<MouseEnteredEvent>([this](MouseEnteredEvent& e) -> bool
    {
      //if (e.hasEntered())
      //{
      //  glfwSetInputMode(m_window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
      //}
      //else
      //{
      //  glfwSetInputMode(m_window->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      //}
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
