#include <glad/glad.h>
#include <iostream>
#include "../window/window.h" 

namespace kogayonon {

  Window::Window(const WindowProps& props) {
    setupWindow(props);
  }

  Window::~Window() {}

  void Window::onUpdate() {}

  unsigned int Window::getWidth() const {
    return m_data.Width;
  }

  unsigned int Window::getHeight() const {
    return m_data.Height;
  }

  void Window::setVsync() {}

  bool Window::isVsync() {
    return m_data.VSync;
  }

  void Window::setEventCallbackFn(const EventCallbackFn& callback) {
    m_data.EventCallback = callback;
  }

  bool Window::setupWindow(const WindowProps& props) {

    if (!glfwInit()) {
      std::cout << "failed to init glfw\n";
      return false;
    }

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

    m_window = glfwCreateWindow(props.Width, props.Height, props.Title, NULL, NULL);
    std::cout << "Window props:" << props.Width << " " << props.Height << " " << props.Title << "\n";
    if (!m_window) {
      std::cout << "failed to create window\n";
      return false;
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
      std::cout << "failed to load glad\n";
      return false;
    }

    return true;
  }

  GLFWwindow* Window::getWindow() {
    return m_window;
  }
}
