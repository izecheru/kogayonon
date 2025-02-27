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

namespace kogayonon
{
  Window::Window()
  {
    init(m_data);
    glfwSetWindowUserPointer(m_window, &m_data);
  }

  Window::~Window()
  {
    Logger::logInfo("~Window destroyed");
    if (m_window)
    {
      glfwDestroyWindow(m_window);
    }
    glfwTerminate();
  }

  void Window::update()
  {
    glfwPollEvents();
    glfwSwapBuffers(m_window);
  }

  unsigned short Window::getWidth() const { return m_data.width; }

  unsigned short Window::getHeight() const { return m_data.height; }

  void Window::setVsync()
  {
    if (!m_data.vsync)
    {
      m_data.vsync = true;
      glfwSwapInterval(1);
    }
    else
    {
      m_data.vsync = false;
      glfwSwapInterval(0);
    }
  }

  bool Window::isVsync() { return m_data.vsync; }

  void Window::setViewport()
  {
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    m_data.width = width;
    m_data.height = height;
    glViewport(0, 0, width, height);
  }

  void Window::setEventCallbackFn(const EventCallbackFn& callback)
  {
    m_data.eventCallback = callback;
  }

  bool Window::init(const window_props& props)
  {
    if (!glfwInit())
    {
      Logger::logError("failed to init glfw\n");
      return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(props.width, props.height, props.title, NULL, NULL);

    // continue only if window is not nullptr
    assert(m_window != nullptr);

    glfwMakeContextCurrent(m_window);

    // continue only if glad can load
    assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress));

    glEnable(GL_DEBUG_OUTPUT);
    /* glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
       GLsizei length, const GLchar* message, const void* userParam)
       {
         auto const src_str = [source]()
           {
             switch (source)
             {
               case GL_DEBUG_SOURCE_API: return "API";
               case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return "WINDOW SYSTEM";
               case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
               case GL_DEBUG_SOURCE_THIRD_PARTY: return "THIRD PARTY";
               case GL_DEBUG_SOURCE_APPLICATION: return "APPLICATION";
               case GL_DEBUG_SOURCE_OTHER: return "OTHER";
             }
           }();

         auto const type_str = [type]()
           {
             switch (type)
             {
               case GL_DEBUG_TYPE_ERROR: return "ERROR";
               case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
               case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return "UNDEFINED_BEHAVIOR";
               case GL_DEBUG_TYPE_PORTABILITY: return "PORTABILITY";
               case GL_DEBUG_TYPE_PERFORMANCE: return "PERFORMANCE";
               case GL_DEBUG_TYPE_MARKER: return "MARKER";
               case GL_DEBUG_TYPE_OTHER: return "OTHER";
             }
           }();

         auto const severity_str = [severity]()
           {
             switch (severity)
             {
               case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
               case GL_DEBUG_SEVERITY_LOW: return "LOW";
               case GL_DEBUG_SEVERITY_MEDIUM: return "MEDIUM";
               case GL_DEBUG_SEVERITY_HIGH: return "HIGH";
             }
           }();
         std::cout << src_str << ", " << type_str << ", " << severity_str << ", " << id << ": " << message << '\n';
       }, nullptr);*/

    glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
      {
        window_props& props = *(window_props*)glfwGetWindowUserPointer(window);
        WindowResizeEvent event(width, height);
        props.eventCallback(event);
      });

    glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double x_pos, double y_pos)
      {
        window_props& props = *(window_props*)glfwGetWindowUserPointer(window);
        MouseMovedEvent event(x_pos, y_pos);
        props.eventCallback(event);
      });

    glfwSetCursorEnterCallback(m_window, [](GLFWwindow* window, int entered)
      {
        window_props& props = *(window_props*)glfwGetWindowUserPointer(window);
        MouseEnteredEvent event(entered);
        props.eventCallback(event);
      });

    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOff, double yOff) { });

    glfwSetMouseButtonCallback(m_window, [](GLFWwindow* window, int button, int action, int mods)
      {
        window_props& props = *(window_props*)glfwGetWindowUserPointer(window);
        MouseClickedEvent event(button, action, mods);
        props.eventCallback(event);
      });

    glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window)
      {
        window_props& props = *(window_props*)glfwGetWindowUserPointer(window);
        WindowCloseEvent event;
        props.eventCallback(event);
      });

    glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scan_code, int action, int mods)
      {
        window_props& props = *(window_props*)glfwGetWindowUserPointer(window);

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
        window_props& props = *(window_props*)glfwGetWindowUserPointer(window);
        KeyTypedEvent event((KeyCode)key_code);
        props.eventCallback(event);
      });

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    glViewport(0, 0, props.width, props.height);
    return true;
  }

  GLFWwindow* Window::getWindow() { return m_window; }

  window_props& Window::getWindowData() { return m_data; }
}