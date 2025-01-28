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

using std::cout;

App::App() {
  m_window = new Window();
  m_renderer = new Renderer();
}

void App::run() {
  std::vector<Vertex> vertices = {
    // Cube 1 (centered at origin)
    // Front face
    {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)}, // Bottom-left
    {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},  // Bottom-right
    {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},   // Top-right
    {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},  // Top-left

    // Back face
    {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)}, // Bottom-left
    {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(1.0f, 0.0f)},  // Bottom-right
    {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},   // Top-right
    {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec2(0.0f, 1.0f)},  // Top-left

    // Top face
    {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.0f, 1.0f, 1.0f), glm::vec2(0.0f, 1.0f)},  // Back-left
    {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)},   // Back-right
    {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec2(1.0f, 0.0f)},    // Front-right
    {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec2(0.0f, 0.0f)},   // Front-left

    // Bottom face
    {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.0f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f)}, // Back-left
    {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.0f, 0.5f), glm::vec2(1.0f, 1.0f)},  // Back-right
    {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.0f), glm::vec2(1.0f, 0.0f)},   // Front-right
    {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.5f, 0.0f), glm::vec2(0.0f, 0.0f)},  // Front-left

    // Left face
    {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)}, // Back-bottom
    {glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.5f, 0.0f), glm::vec2(1.0f, 0.0f)},  // Front-bottom
    {glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(0.0f, 0.0f, 0.5f), glm::vec2(1.0f, 1.0f)},   // Front-top
    {glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(0.0f, 1.0f)},  // Back-top

    // Right face
    {glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, 0.0f, 0.5f), glm::vec2(0.0f, 0.0f)},  // Back-bottom
    {glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.0f, 0.5f, 0.5f), glm::vec2(1.0f, 0.0f)},   // Front-bottom
    {glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.0f), glm::vec2(1.0f, 1.0f)},    // Front-top
    {glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(1.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)},   // Back-top
  };
  // Cube indices (for both cubes)
  std::vector<unsigned int> indices = {
    // Front face
    0, 1, 2, 2, 3, 0,
    // Back face
    4, 5, 6, 6, 7, 4,
    // Top face
    8, 9, 10, 10, 11, 8,
    // Bottom face
    12, 13, 14, 14, 15, 12,
    // Left face
    16, 17, 18, 18, 19, 16,
    // Right face
    20, 21, 22, 22, 23, 20
  };
  glEnable(GL_DEPTH_TEST);
  m_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });
  m_renderer->pushShader("H:/Git/kogayonon/shaders/3d_vertex.glsl", "H:/Git/kogayonon/shaders/3d_fragment.glsl", "3d_shader");

  double prev_time = glfwGetTime();

  Mesh cube("H:/Git/kogayonon/textures/pintea.png", vertices, indices);
  m_renderer->pushMesh("cube", cube);
  Camera& camera = Camera::getInstance();
  glfwSwapInterval(1);
  float rotation = 0.0f;
  while (!glfwWindowShouldClose(m_window->getWindow()))
  {
    camera.processKeyboard(m_window->getWindow(), delta_time);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // Dark gray background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    float current_time = glfwGetTime();
    delta_time = current_time - prev_time;
    prev_time = current_time;
    if (current_time - prev_time >= 1 / 60)
    {
      rotation += 0.5f;
      prev_time = current_time;
    }

    m_renderer->bindShader("3d_shader");
    glm::mat4 model = glm::mat4(1.0f);
    // glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 proj = glm::mat4(1.0f);
    float scaleFactor = 0.40f;  // Example scale factor
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, scaleFactor));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(1.0f, 0.0f, 1.0f));
    proj = glm::perspective(glm::radians(45.0f), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 100.0f);

    // Outputs the matrices into the Vertex Shader
    int modelLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    camera.cameraUniform(m_renderer->getShaderId("3d_shader"), "view");

    int projLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
    int scaleLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "scaleMatrix");
    glUniformMatrix4fv(scaleLoc, 1, GL_FALSE, glm::value_ptr(scaleMatrix));
    m_renderer->render("cube");

    m_window->update();
  }

  // Cleanup

  delete m_window;
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
      if (e.getKeyCode() == KeyCode::F1)
      {
        glfwSetWindowShouldClose(m_window->getWindow(), true);
      }
      return this->onKeyPress(e);
    });

  dispatcher.dispatch<MouseEnteredEvent>([this](MouseEnteredEvent& e) -> bool
    {
      if (e.hasEntered())
      {
        glfwSetInputMode(m_window->getWindow(), GLFW_CURSOR,
          GLFW_CURSOR_DISABLED);
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
