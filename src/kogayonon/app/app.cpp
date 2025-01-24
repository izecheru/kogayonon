#include "shader/shader.h"
#include <renderer/mesh.h>
#include "renderer/buffer.h"
#include <glfw/glfw3.h>
#include <iostream>
#include "app/app.h"
#include "window/window.h"
#include "events/keyboard_events.h"
#include "core/logger.h"
#include "renderer/renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using std::cout;

namespace kogayonon
{
  App::App() {
    m_window = new Window();
    m_renderer = new Renderer();
  }

  void App::run() {
    std::vector<Vertex> vertices = {
     { glm::vec3(-0.5f, 0.0f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f)},
     { glm::vec3(-0.5f, 0.0f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f)},
     { glm::vec3(0.5f, 0.0f, -0.5f), glm::vec3(0.83f, 0.70f, 0.44f)},
     { glm::vec3(0.5f, 0.0f, 0.5f), glm::vec3(0.83f, 0.70f, 0.44f)},
     { glm::vec3(0.0f, 0.8f, 0.0f), glm::vec3(0.92f, 0.86f, 0.76f)}
    };

    std::vector<unsigned int >indices
    {
      0, 1, 2,
      0, 2, 3,
      0, 1, 4,
      1, 2, 4,
      2, 3, 4,
      3, 0, 4
    };

    glEnable(GL_DEPTH_TEST);
    m_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });
    m_renderer->pushShader("D:\\repos\\kogayonon\\shaders\\3d.shader", "3d_shader");

    GLuint uniID = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "scale");
    float rotation = 0.0f;
    double prevTime = glfwGetTime();

    Mesh piramid(vertices, indices);

    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark gray background
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      double crntTime = glfwGetTime();
      if (crntTime - prevTime >= 1 / 60)
      {
        rotation += 0.5f;
        prevTime = crntTime;
      }
      m_renderer->bindShader("3d_shader");
      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 view = glm::mat4(1.0f);
      glm::mat4 proj = glm::mat4(1.0f);
      float scaleFactor = 2.0f; // Example scale factor
      glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor, scaleFactor, scaleFactor));

      // Assigns different transformations to each matrix
      model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
      view = glm::translate(view, glm::vec3(0.0f, -0.5f, -2.0f));
      proj = glm::perspective(glm::radians(60.0f), (float)m_window->getWidth() / (float)m_window->getHeight(), 0.1f, 100.0f);

      // Outputs the matrices into the Vertex Shader
      int modelLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "model");
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      int viewLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "view");
      glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
      int projLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "proj");
      glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
      glUniform1f(uniID, 2.0f);

      piramid.draw();
      m_window->update();
    }

    // Cleanup

    delete m_window;
  }

  void App::onEvent(Event& event) {
    //Logger::logInfo(event.toString());
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e)->bool
      {
        Logger::logError("[dispatch updates on resize]\n");
        return this->onWindowResize(e);
      });

    dispatcher.dispatch<WindowCloseEvent>([this](WindowCloseEvent& e)->bool
      {
        Logger::logError("[dispatch updates on close]\n");
        return this->onWindowClose(e);
      });
  }

  bool App::onWindowResize(WindowResizeEvent& event) {
    m_window->setViewport(event.getWidth(), event.getHeight());
    return true;
  }

  bool App::onWindowClose(WindowCloseEvent& event) {
    return true;
  }

  GLFWwindow* App::getWindow() {
    return m_window->getWindow();
  }
  void App::scrollCallback(GLFWwindow* window, double x, double y) {}
}  // namespace kogayonon
