#include <glfw3.h>
#include <iostream>
#include "app.h"
#include "../event/event.h"
#include "../event/mouse/mouse_events.h"
#include "../window/window.h"
using std::cout;

namespace kogayonon
{

  void kogayonon::App::mainLoop()
  {
    Window* my_window = new kogayonon::Window();
    my_window->setEventCallbackFn([this](Event& e) -> void { this->onEvent(e); });


    glfwSetWindowSizeCallback(my_window->getWindow(), [](GLFWwindow* window, int width, int height)
      {
        WindowProps& data = *(WindowProps*)glfwGetWindowUserPointer(window);
        data.m_width = width;
        data.m_height = height;
        WindowResizeEvent event(width, height);
        data.m_event_callback(event);
      });

    glfwSetCursorPosCallback(my_window->getWindow(), [](GLFWwindow* window, double x_pos, double y_pos)
      {
        WindowProps& data = *(WindowProps*)glfwGetWindowUserPointer(window);
        MouseMovedEvent event(x_pos, y_pos);
        data.m_event_callback(event);
      });

    while (!glfwWindowShouldClose(my_window->getWindow()))
    {
      int width = 0, height = 0;
      glfwGetFramebufferSize(my_window->getWindow(), &width, &height);
      glClear(GL_COLOR_BUFFER_BIT);

      glViewport(0, 0, 640, 480);

      glfwSwapBuffers(my_window->getWindow());
      glfwPollEvents();
    }

    glfwDestroyWindow(my_window->getWindow());
    glfwTerminate();
  }

  void App::onEvent(Event& event)
  {
    std::cout << event.toString() << std::endl;
    EventDispatcher dispatcher(event);
    dispatcher.dispatch<WindowResizeEvent>([this](WindowResizeEvent& e)->bool
      {
        return this->onWindowResize(e);
      });
  }
  bool App::onWindowResize(WindowResizeEvent& event)
  {
    std::cout << event.getWidth() << " " << event.getHeight() << "\n";
    return true;
  }
}  // namespace kogayonon
