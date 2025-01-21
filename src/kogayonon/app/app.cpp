#include "shader/shader.h"
#include <glfw3.h>
#include <iostream>
#include "app/app.h"
#include "window/window.h"
#include "events/keyboard_events.h"
#include "core/logger.h"

using std::cout;

namespace kogayonon
{

  App::App()
  {
    m_window = new Window();
  }

  void App::run()
  {
    // TODO: i must setup the render::init here and the imgui layer
    m_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });
    Shader sh("shaders/Basic.shader");


    GLfloat vertices[] = {
           -0.7f, -0.7f, 0.0f,  // Bottom-left
     0.7f, -0.7f, 0.0f,  // Bottom-right
    -0.7f,  0.7f, 0.0f,  // Top-left

    // Second Triangle
     0.7f, -0.7f, 0.0f,  // Bottom-right
     0.7f,  0.7f, 0.0f,  // Top-right
    -0.7f,  0.7f, 0.0f   // Top-left    
    };

    GLuint vertex_blueprint, vertex_storage;


     // Create and bind VAO
    glGenVertexArrays(1, &vertex_blueprint);
    glBindVertexArray(vertex_blueprint);

    // Create and bind VBO
    glGenBuffers(1, &vertex_storage);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_storage);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Define the vertex attribute layout
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO and VBO for safety
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    while (!glfwWindowShouldClose(m_window->getWindow()))
    {
      glClear(GL_COLOR_BUFFER_BIT); // Clear the screen

      glClear(GL_COLOR_BUFFER_BIT); // Clear the screen

      sh.bind();                   // Use the shader program
      glBindVertexArray(vertex_blueprint);      // Bind the VAO (contains attribute state)
      glDrawArrays(GL_TRIANGLES, 0, 6); // Draw the triangle
      glBindVertexArray(0);        // Unbind VAO for safety

      m_window->onUpdate();        // Swap buffers and poll events
    }

    // Cleanup
    sh.unbind();
    glDeleteBuffers(1, &vertex_storage);
    glDeleteVertexArrays(1, &vertex_blueprint);
    delete m_window;
  }

  void App::onEvent(Event& event)
  {
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

  bool App::onWindowResize(WindowResizeEvent& event)
  {
    glViewport(0, 0, event.getWidth(), event.getHeight());
    return true;
  }

  bool App::onWindowClose(WindowCloseEvent& event)
  {

    return true;
  }
}  // namespace kogayonon
