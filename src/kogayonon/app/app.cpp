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
    m_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });
    m_renderer->pushShader("shaders/3d.shader", "3d_shader");


    GLfloat vertices[] =
    { //     COORDINATES     /        COLORS      /   TexCoord  //
      -0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	0.0f, 0.0f,
      -0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	5.0f, 0.0f,
       0.5f, 0.0f, -0.5f,     0.83f, 0.70f, 0.44f,	0.0f, 0.0f,
       0.5f, 0.0f,  0.5f,     0.83f, 0.70f, 0.44f,	5.0f, 0.0f,
       0.0f, 0.8f,  0.0f,     0.92f, 0.86f, 0.76f,	2.5f, 5.0f
    };

    GLuint indices[] =
    {
      0, 1, 2,
      0, 2, 3,
      0, 1, 4,
      1, 2, 4,
      2, 3, 4,
      3, 0, 4
    };

    VertexArrayBuffer vao;
    VertexBuffer my_buffer(vertices, sizeof(vertices));
    glEnable(GL_DEPTH_TEST);
    ElementArrayBuffer element_buffer(indices, sizeof(indices));
    // Links VBO attributes such as coordinates and colors to VAO
    vao.linkAttrib(my_buffer, 0, 3, GL_FLOAT, 8 * sizeof(float), (void*)0);
    vao.linkAttrib(my_buffer, 1, 3, GL_FLOAT, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    vao.linkAttrib(my_buffer, 2, 2, GL_FLOAT, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    float rotation = 0.0f;
    double prevTime = glfwGetTime();
    while (!glfwWindowShouldClose(m_window->getWindow())) {
      glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
      // Clean the back buffer and depth buffer
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      double crntTime = glfwGetTime();
      if (crntTime - prevTime >= 1 / 60) {
        rotation += 0.5f;
        prevTime = crntTime;
      }
      m_renderer->bindShader("3d_shader");
      vao.bind();
      glDrawArrays(GL_TRIANGLES, 0, 12);
      vao.unbind();
      glm::mat4 model = glm::mat4(1.0f);
      glm::mat4 view = glm::mat4(1.0f);
      glm::mat4 proj = glm::mat4(1.0f);

      // Assigns different transformations to each matrix
      model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 1.0f, 0.0f));
      view = glm::translate(view, glm::vec3(0.0f, -0.5f, -2.0f));
      proj = glm::perspective(glm::radians(45.0f), (float)m_window->getWidth() / m_window->getHeight(), 0.1f, 100.0f);

      // Outputs the matrices into the Vertex Shader
      int modelLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "model");
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      int viewLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "view");
      glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
      int projLoc = glGetUniformLocation(m_renderer->getShaderId("3d_shader"), "proj");
      glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));
      m_window->update();        // Swap buffers and poll events
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
}  // namespace kogayonon
