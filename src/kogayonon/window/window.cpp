#include <glad/glad.h>
#include <iostream>
#include "../window/window.h" 

namespace kogayonon
{

  Window::Window()
  {
    setupWindow(m_data);
    glfwSetWindowUserPointer(m_window, &m_data);
  }

  Window::~Window() {}

  void Window::onUpdate() {}

  void Window::onClose()
  {}

  unsigned int Window::getWidth() const
  {
    return m_data.m_width;
  }

  unsigned int Window::getHeight() const
  {
    return m_data.m_height;
  }

  void Window::setVsync() {}

  bool Window::isVsync()
  {
    return m_data.m_vsync;
  }

  void Window::setEventCallbackFn(const EventCallbackFn& callback)
  {
    m_data.m_event_callback = callback;
  }

  bool Window::setupWindow(const WindowProps& props)
  {

    if (!glfwInit())
    {
      std::cout << "failed to init glfw\n";
      return false;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    m_window = glfwCreateWindow(props.m_width, props.m_height, props.m_title, NULL, NULL);
    std::cout << "Window props:" << props.m_width << " " << props.m_height << " " << props.m_title << "\n";
    if (!m_window)
    {
      std::cout << "failed to create window\n";
      return false;
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
      std::cout << "failed to load glad\n";
      return false;
    }

    return true;
  }

  GLFWwindow* Window::getWindow()
  {
    return m_window;
  }

  WindowProps Window::getWindowData()
  {
    return m_data;
  }
}
