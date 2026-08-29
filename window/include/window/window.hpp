#pragma once
#include <SDL2/SDL.h>
#include <functional>
#include <memory>
#include <sol/sol.hpp>

namespace window
{
struct WindowProperties
{
  const char* title;

  int width{ 0 };
  int height{ 0 };

  bool vsync{ false };
  bool maximized{ false };

  int x{ 0 };
  int y{ 0 };

  explicit WindowProperties( const char* t_title, int t_width, int t_height, bool t_vsync, bool max )
      : title{ t_title }
      , width{ t_width }
      , height{ t_height }
      , vsync{ t_vsync }
      , maximized{ max }
  {
  }

  ~WindowProperties() = default;
};

class Window
{
public:
  explicit Window( const char* t_title, int t_width, int t_height, bool t_vsync, bool t_maximized );
  ~Window();

  /**
   * @brief Maximize window
   */
  void maximize();

  /**
   * @brief Is maximized?
   * @return
   */
  bool getMaximized();

  /**
   * @brief Get window width
   * @return
   */
  int getWidth() const;

  /**
   * @brief Get window height
   * @return
   */
  int getHeight() const;

  /**
   * @brief Set window width
   * @param w
   */
  void setWidth( int w );

  /**
   * @brief Set window height
   * @param h
   */
  void setHeight( int h );
  void resize();

  /**
   * @brief Resize the window
   * @param w
   * @param h
   */
  void resize( int w, int h );

  /**
   * @brief Moves window at screen coordinates
   * @param x
   * @param y
   */
  void placeAt( int x, int y );

  /**
   * @brief Set window title
   * @param title
   */
  void setTitle( const char* title );

  /**
   * @brief Flags as bordered
   * @param value
   */
  void setBordered( bool value );

  /**
   * @brief Flags as resizable
   * @param value
   */
  void setResizable( bool value );

  /**
   * @brief Centers window on screen
   */
  void centerWindow();

  auto getWindowProps() -> WindowProperties*;

  auto getWindow() -> SDL_Window*;
  void setWindow( SDL_Window* wnd );

  static void createLuaBindings( sol::state& lua );

private:
  SDL_Window* m_window = nullptr;
  std::shared_ptr<WindowProperties> m_pWindowProps;
  bool m_initialized = false;
};
} // namespace window
