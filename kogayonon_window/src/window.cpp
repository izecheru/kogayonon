#include "window/window.hpp"

#include <assert.h>

namespace kogayonon_window
{
Window::Window( const char* t_title, int t_width, int t_height, bool t_vsync, bool t_maximized )
    : m_pWindowProps( std::make_shared<window_props>( t_title, t_width, t_height, t_vsync, t_maximized ) )
{
  int x, y;
  SDL_GetWindowPosition( m_window, &x, &y );
  m_pWindowProps->x = x;
  m_pWindowProps->y = y;
}

Window::~Window()
{
  if ( m_window )
  {
    SDL_DestroyWindow( m_window );
    m_window = nullptr;
  }

  if ( m_glContext )
  {
    SDL_GL_DeleteContext( m_glContext );
    m_glContext = nullptr;
  }
  SDL_Quit();
}

void Window::swapWindow()
{
  SDL_GL_SwapWindow( m_window );
}

void Window::setWidth( int w )
{
  m_pWindowProps->width = w;
}

void Window::setHeight( int h )
{
  m_pWindowProps->height = h;
}

void Window::resize()
{
  int w, h;
  SDL_GL_GetDrawableSize( m_window, &w, &h );
  setWidth( w );
  setHeight( h );
}

void Window::resize( int w, int h )
{
  setWidth( w );
  setHeight( h );
  SDL_SetWindowSize( m_window, w, h );
}

void Window::placeAt( int x, int y )
{
  SDL_SetWindowPosition( m_window, x, y );
  m_pWindowProps->x = x;
  m_pWindowProps->y = y;
}

void Window::setTitle( const char* title )
{
  SDL_SetWindowTitle( m_window, title );
}

void Window::setBordered( bool value )
{
  SDL_SetWindowBordered( m_window, value == true ? SDL_TRUE : SDL_FALSE );
}

void Window::setResizable( bool value )
{
  SDL_SetWindowResizable( m_window, value == true ? SDL_TRUE : SDL_FALSE );
}

bool Window::getMaximized()
{
  return m_pWindowProps->maximized;
}

void Window::maximize()
{
  m_pWindowProps->maximized = true;
  SDL_MaximizeWindow( m_window );
  resize();
}

int Window::getWidth() const
{
  return m_pWindowProps->width;
}

int Window::getHeight() const
{
  return m_pWindowProps->height;
}

SDL_Window* Window::getWindow()
{
  return m_window;
}

SDL_GLContext Window::getContext() const
{
  return m_glContext;
}

void Window::setContext( SDL_GLContext ctx )
{
  assert( ctx != nullptr && "SDL_GLContext can't be nullptr" );
  m_glContext = ctx;
}

void Window::setWindow( SDL_Window* wnd )
{
  assert( wnd != nullptr && "SDL_Window* can't be nullptr" );
  m_window = wnd;
}

window_props* Window::getWindowProps()
{
  return m_pWindowProps.get();
}
} // namespace kogayonon_window