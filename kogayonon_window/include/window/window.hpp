#pragma once
#include <SDL2/SDL.h>
#include <functional>
#include <memory>

namespace kogayonon_window
{
struct window_props
{
  const char* title;
  int width;
  int height;
  bool vsync;
  bool maximized;
  int x;
  int y;

  explicit window_props( const char* t_title, int t_width, int t_height, bool t_vsync, bool max )
      : title{ t_title }
      , width{ t_width }
      , height{ t_height }
      , vsync{ t_vsync }
      , maximized{ max }
  {
  }

  ~window_props() = default;
};

class Window
{
public:
  explicit Window( const char* t_title, int t_width, int t_height, bool t_vsync, bool t_maximized );
  ~Window();

  void swapWindow();

  void setMaximized( bool value );
  bool getMaximized();

  int getWidth() const;
  int getHeight() const;
  void setWidth( int w );
  void setHeight( int h );
  void resize();
  void resize( int w, int h );
  void placeAt( int x, int y );

  window_props* getWindowProps();

  SDL_Window* getWindow();
  void setWindow( SDL_Window* wnd );

  SDL_GLContext getContext() const;
  void setContext( SDL_GLContext ctx );

private:
  SDL_Window* m_window = nullptr;
  std::shared_ptr<window_props> m_pWindowProps;
  bool m_initialized = false;
  SDL_GLContext m_glContext = nullptr;
};
} // namespace kogayonon_window
