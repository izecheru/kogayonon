#pragma once
#include <functional>
#include <imgui.h>
#include <memory>
#include <string>

namespace kogayonon_gui
{
class ImGuiWindow
{
public:
  explicit ImGuiWindow( std::string name );
  virtual ~ImGuiWindow();
  virtual void draw() = 0;

  std::string getName() const;

  virtual void setDocked( bool status );
  virtual void setVisible( bool status );
  virtual void setX( double x );
  virtual void setY( double y );

  /**
   * @brief Sets up width, height, x, y when called
   * @param dirty If dirty is true it will set proportions every frame, false is for first setup
   */
  virtual void setupProportions( bool dirty = true );

  int width();
  int height();

protected:

  // if we ever need to pass a rendering func in the middle of the window or something
  // i made this for the scene viewport so i can pass functions from Renderer class

  struct imgui_props
  {
    std::string name;
    double x{ 0.0 };
    double y{ 0.0 };
    int width{ 0 };
    int height{ 0 };
    bool canMove{ true };
    bool visible{ true };
    bool docked{ false };
    bool hovered{ false };
    bool resizable{ false };
    bool focused{ false };
    ImGuiWindowFlags flags{ 0 };

    explicit imgui_props( std::string t_name )
        : name{ std::move( t_name ) }
    {
    }
  };

  std::unique_ptr<imgui_props> m_props = nullptr;
};
} // namespace kogayonon_gui