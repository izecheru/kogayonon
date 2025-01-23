#include "shader/shader.h"
#include "renderer/vertex_array_buffer.h"
#include <glfw3.h>
#include "renderer/vertex_buffer.h"
#include <iostream>
#include "app/app.h"
#include "window/window.h"
#include "events/keyboard_events.h"
#include "core/logger.h"
#include "renderer/renderer.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include "renderer/element_array_buffer.h"

using std::cout;

namespace kogayonon
{
  App::App() {
    m_window = new Window();
    m_renderer = new Renderer();
  }

  void App::run() {


    GLfloat vertices[] =
    {
      //     COORDINATES        /      COLORS         /   TexCoord  //
      -0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,    0.0f, 0.0f,   // Triangle 1 (Color 1)
      -0.5f, 0.0f, -0.5f,     0.99f, 0.44f, 0.33f,    5.0f, 0.0f,   // Triangle 1 (Color 1)
       0.5f, 0.0f, -0.5f,     0.50f, 0.33f, 0.99f,    0.0f, 0.0f,   // Triangle 1 (Color 1)

       0.5f, 0.0f,  0.5f,     0.33f, 0.83f, 0.44f,    5.0f, 0.0f,   // Triangle 2 (Color 2)
       0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,    2.5f, 5.0f,   // Triangle 2 (Color 2)
      -0.5f, 0.0f,  0.5f,     0.10f, 0.99f, 0.92f,    0.0f, 0.0f,   // Triangle 2 (Color 2)

       0.5f, 0.0f, -0.5f,     0.70f, 0.44f, 0.83f,    0.0f, 0.0f,   // Triangle 3 (Color 3)
       0.0f, 0.8f,  0.0f,     0.99f, 0.75f, 0.10f,    2.5f, 5.0f,   // Triangle 3 (Color 3)
       0.5f, 0.0f,  0.5f,     0.10f, 0.50f, 0.99f,    5.0f, 0.0f,   // Triangle 3 (Color 3)
    };
    unsigned int indices[] =
    {
      0, 1, 2,
      0, 2, 3,
      0, 1, 4,
      1, 2, 4,
      2, 3, 4,
      3, 0, 4
    };

#define MYFUNC
    glEnable(GL_DEPTH_TEST);
    m_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });
    m_renderer->pushShader("shaders/3d.shader", "3d_shader");

#ifdef MYFUNC
    m_renderer->bindVao();
    VertexBuffer vbo(vertices, sizeof(vertices));
    ElementArrayBuffer ebo(indices, sizeof(indices));
    VertexArrayBuffer vao = m_renderer->getVao();

    vao.linkAttrib(vbo, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    vao.linkAttrib(vbo, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    vao.linkAttrib(vbo, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    vao.unbind();
    ebo.unbind();
    vbo.unbind();
#endif
    GLuint uniID = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "scale");
    float rotation = 0.0f;
    double prevTime = glfwGetTime();

    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(m_window->getWindow())) {
      glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark gray background
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      double crntTime = glfwGetTime();
      if (crntTime - prevTime >= 1 / 60) {
        rotation += 0.5f;
        prevTime = crntTime;
      }
      m_renderer->bindShader("3d_shader");
      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 view = glm::mat4(1.0f);
      glm::mat4 proj = glm::mat4(1.0f);

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

#ifdef MYFUNC
      vao.bind();
      glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(GLuint), GL_UNSIGNED_INT, 0);
      vao.unbind();
#endif
      m_window->update();
    }

    // Cleanup
    m_renderer->unbindShaders();
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
