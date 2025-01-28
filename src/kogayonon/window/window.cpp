#include <glad/glad.h>

#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif


#include "window/window.h"
#include "core/input/input.h"
#include "core/logger.h"
#include "events/app_event.h"
#include "events/keyboard_events.h"
#include "events/mouse_events.h"

std::unordered_set<KeyCode> keys_pressed;

Window::Window() {
  init(m_data);
  glfwSetWindowUserPointer(m_window, &m_data);
}

Window::~Window() {
  if (m_window)
  {
    glfwDestroyWindow(m_window);
  }
  glfwTerminate();
}

void Window::update() {
  glfwPollEvents();
  glfwSwapBuffers(m_window);
}

void Window::onClose() { glfwDestroyWindow(m_window); }

unsigned int Window::getWidth() const { return m_data.m_width; }

unsigned int Window::getHeight() const { return m_data.m_height; }

void Window::setVsync(bool enabled) {
  if (enabled)
  {
    glfwSwapInterval(1);
  }
  else
  {
    glfwSwapInterval(0);
  }
  m_data.m_vsync = enabled;
}

bool Window::isVsync() { return m_data.m_vsync; }

void Window::setViewport(int width, int height) {
  glViewport(0, 0, width, height);
}

void Window::setViewport() {
  glViewport(0, 0, m_data.m_width, m_data.m_height);
}

void Window::setEventCallbackFn(const EventCallbackFn& callback) {
  m_data.eventCallback = callback;
}

bool Window::init(const WindowProps& props) {
  if (!glfwInit())
  {
    Logger::logError("failed to init glfw\n");
    return false;
  }

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

  m_window = glfwCreateWindow(props.m_width, props.m_height, props.m_title,
    NULL, NULL);
  if (!m_window)
  {
    Logger::logError("failed to create window\n");
    return false;
  }

  glfwMakeContextCurrent(m_window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    Logger::logError("failed to load glad\n");
    return false;
  }

  glfwSetWindowSizeCallback(
    m_window, [](GLFWwindow* window, int width, int height)
    {
      WindowProps& props = *(WindowProps*)glfwGetWindowUserPointer(window);
      WindowResizeEvent event(width, height);
      props.eventCallback(event);
    });

  glfwSetCursorPosCallback(
    m_window, [](GLFWwindow* window, double x_pos, double y_pos)
    {
      WindowProps& props = *(WindowProps*)glfwGetWindowUserPointer(window);
      MouseMovedEvent event(x_pos, y_pos);
      props.eventCallback(event);
    });

  glfwSetCursorEnterCallback(m_window, [](GLFWwindow* window, int entered)
    {
      WindowProps& props = *(WindowProps*)glfwGetWindowUserPointer(window);
      MouseEnteredEvent event(entered);
      props.eventCallback(event);
    });

  glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOff, double yOff) {});

  glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
    {
      WindowProps& props = *(WindowProps*)glfwGetWindowUserPointer(window);
      MouseClickedEvent event(button, action, mods);
      props.eventCallback(event);
    });

  glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
    {
      WindowProps& props = *(WindowProps*)glfwGetWindowUserPointer(window);
      WindowCloseEvent event;
      props.eventCallback(event);
    });

  glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scan_code, int action, int mods)
    {
      WindowProps& props = *(WindowProps*)glfwGetWindowUserPointer(window);

      switch (action)
      {
        case GLFW_PRESS:
          {
            KeyPressedEvent event((KeyCode)key, 0);
            props.eventCallback(event);
            break;
          }
        case GLFW_RELEASE:
          {
            KeyReleasedEvent event((KeyCode)key);
            props.eventCallback(event);
            break;
          }
        case GLFW_REPEAT:
          {
            KeyPressedEvent event((KeyCode)key, 1);
            props.eventCallback(event);
          }
      }
    });

  glfwSetCharCallback(m_window, [](GLFWwindow* window, unsigned int key_code)
    {
      WindowProps& props = *(WindowProps*)glfwGetWindowUserPointer(window);
      KeyTypedEvent event((KeyCode)key_code);
      props.eventCallback(event);
    });

  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  setViewport();
  return true;
}

GLFWwindow* Window::getWindow() { return m_window; }

WindowProps Window::getWindowData() { return m_data; }
