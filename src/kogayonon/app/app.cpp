#include "app.h" 
#include "../event/mouse/mouse_events.h"
#include "../window/window.h"
#include "../event/event.h"
#include <glfw3.h> 
#include <iostream>
using std::cout;


namespace kogayonon
{

  void kogayonon::App::mainLoop()
  {
    WindowProps props;
    Window* my_window = new kogayonon::Window(props);

    glfwSetWindowUserPointer(my_window->getWindow(), &props);

    my_window->setEventCallbackFn([this](Event& e)
    {
      this->onEvent(e);
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
    std::cout << "onEvent called for event type: " << event.toString() << std::endl;
    EventDispatcher dispatcher(event);
    dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e)
    {
      this->onWindowResize(e);
    });
    std::cout << "on event called";
  }

  void kogayonon::App::onWindowResize(WindowResizeEvent& event)
  {
    std::cout << "on window resize";
  }
}
