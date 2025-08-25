#include "window/window.h"

#include <imgui_impl_sdl2.h>

#include "context_manager/context_manager.h"
#include "event/app_event.h"
#include "input/keyboard_events.h"
#include "input/mouse_events.h"
#include "klogger/klogger.h"

namespace kogayonon
{
Window::Window()
{
  init(m_data);
}

Window::~Window()
{
  ContextManager::klogger()->log(LogType::INFO, "~Window destroyed");
  if (m_gl_context != nullptr)
    SDL_GL_DeleteContext(m_gl_context);

  if (m_window != nullptr)
    SDL_DestroyWindow(m_window);

  m_window = nullptr;
  SDL_Quit();
}

void Window::update()
{
  SDL_Event event;
  while (SDL_PollEvent(&event))
  {
    ImGui_ImplSDL2_ProcessEvent(&event);
    switch (event.type)
    {
    case SDL_QUIT: {
      WindowCloseEvent e;
      m_data.callback(e);
      break;
    }
    case SDL_WINDOWEVENT: {
      if (event.window.event == SDL_WINDOWEVENT_RESIZED)
      {
        m_data.width = event.window.data1;
        m_data.height = event.window.data2;
        WindowResizeEvent e(m_data.width, m_data.height);
        m_data.callback(e);
      }
      break;
    }
    case SDL_MOUSEMOTION: {
      MouseMovedEvent e(event.motion.x, event.motion.y);
      m_data.callback(e);
      break;
    }
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
      MouseClickedEvent e(event.button.button, event.button.state, event.button.clicks);
      m_data.callback(e);
      break;
    }
    case SDL_MOUSEWHEEL: {
      MouseScrolledEvent e(event.wheel.x, event.wheel.y);
      m_data.callback(e);
      break;
    }
    case SDL_KEYDOWN: {
      KeyPressedEvent e((KeyCode)event.key.keysym.sym, event.key.repeat);
      m_data.callback(e);
      break;
    }
    case SDL_KEYUP: {
      KeyReleasedEvent e((KeyCode)event.key.keysym.sym);
      m_data.callback(e);
      break;
    }
    case SDL_TEXTINPUT: {
      KeyTypedEvent e((KeyCode)event.text.text[0]);
      m_data.callback(e);
      break;
    }
    }
  }

  SDL_GL_SwapWindow(m_window);
}

unsigned short Window::getWidth() const
{
  return m_data.width;
}

unsigned short Window::getHeight() const
{
  return m_data.height;
}

void Window::setVsync()
{
  m_data.vsync != m_data.vsync;
  SDL_GL_SetSwapInterval(m_data.vsync ? 1 : 0);
}

bool Window::isVsync()
{
  return m_data.vsync;
}

void Window::setViewport()
{
  int w, h;
  SDL_GL_GetDrawableSize(m_window, &w, &h);
  m_data.width = w;
  m_data.height = h;
  glViewport(0, 0, w, h);
}

void Window::maximize()
{
  SDL_MaximizeWindow(m_window);
}

bool Window::init(const window_props& props)
{
  if (SDL_Init(SDL_INIT_VIDEO) < 0)
  {
    ContextManager::klogger()->log(LogType::ERROR, "failed to init sdl\n");
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  m_window = SDL_CreateWindow(props.title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, props.width, props.height,
                              SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_MAXIMIZED);

  // continue only if window is not nullptr
  assert(m_window != nullptr);

  m_gl_context = SDL_GL_CreateContext(m_window);
  assert(m_gl_context != nullptr);
  SDL_GL_MakeCurrent(m_window, m_gl_context);

  // continue only if glad can load
  assert(gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress));

  glEnable(GL_DEBUG_OUTPUT);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glViewport(0, 0, props.width, props.height);
  m_data = props;
  m_initialized = true;
  return true;
}

SDL_Window* Window::getWindow()
{
  return m_window;
}

window_props& Window::getWindowData()
{
  return m_data;
}

void Window::setEventCallbackFn(const EventCallbackFn& callback)
{
  m_data.callback = callback;
}
} // namespace kogayonon