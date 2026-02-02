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
  SDL_SetWindowSize( m_window, w, m_pWindowProps->height );
}

void Window::setHeight( int h )
{
  m_pWindowProps->height = h;
  SDL_SetWindowSize( m_window, m_pWindowProps->height, h );
}

void Window::resize()
{
  int w, h;
  SDL_GL_GetDrawableSize( m_window, &w, &h );
  // setWidth( w );
  // setHeight( h );
}

void Window::resize( int w, int h )
{
  m_pWindowProps->width = w;
  m_pWindowProps->height = h;
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

void Window::centerWindow()
{
  SDL_SetWindowPosition( m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED );
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

auto Window::getWidth() const -> int
{
  return m_pWindowProps->width;
}

auto Window::getHeight() const -> int
{
  return m_pWindowProps->height;
}

auto Window::getWindow() -> SDL_Window*
{
  return m_window;
}

auto Window::getContext() const -> SDL_GLContext
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

auto Window::getWindowProps() -> window_props*
{
  return m_pWindowProps.get();
}

void Window::createLuaBindings( sol::state& lua )
{
  lua.new_usertype<Window>(
    "Window",
    "setResizable",
    []( Window& self, bool value ) { return self.setResizable( value ); },
    "setBordered",
    []( Window& self, bool value ) { return self.setBordered( value ); },
    "setTitle",
    []( Window& self, const std::string& title ) { return self.setTitle( title.c_str() ); },
    "resize",
    []( Window& self, int w, int h ) { return self.resize( w, h ); },
    "setHeight",
    []( Window& self, int h ) { return self.setHeight( h ); },
    "setWidth",
    []( Window& self, int w ) { return self.setWidth( w ); },
    "getHeight",
    []( Window& self ) { return self.getHeight(); },
    "getWidth",
    []( Window& self ) { return self.getWidth(); } );
}

} // namespace kogayonon_window