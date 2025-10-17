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
  explicit ImGuiWindow( std::string name, ImGuiWindowFlags flags );
  explicit ImGuiWindow( std::string name, ImGuiWindowFlags flags, ImVec2 size );
  explicit ImGuiWindow( std::string name, ImVec2 size );
  virtual ~ImGuiWindow() = default;

  virtual void draw() = 0;

  std::string getName() const;

  void hide();
  void show();

  virtual void setDocked();
  virtual void updateHovered();
  virtual void updateFocused();

  virtual void updatePosition();
  virtual void updateSize();
  virtual void setBounds();

  virtual bool begin();
  virtual void end();

  /**
   * @brief Sets up width, height, x, y when called
   */
  virtual void initProps();

  int width();
  int height();

protected:
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
    ImVec2 size{ 0.0f, 0.0f };

    struct
    {
      ImVec2 topLeft{ 0.0f, 0.0f };
      ImVec2 bottomRight{ 0.0f, 0.0f };
    } bounds;

    explicit imgui_props( std::string t_name )
        : name{ std::move( t_name ) }
    {
    }

    explicit imgui_props( std::string t_name, ImGuiWindowFlags t_flags )
        : name{ t_name }
        , flags{ t_flags }
    {
    }

    explicit imgui_props( std::string t_name, ImGuiWindowFlags t_flags, ImVec2 size )
        : name{ t_name }
        , flags{ t_flags }
        , size{ size }
    {
    }

    explicit imgui_props( std::string t_name, ImVec2 size )
        : name{ t_name }
        , size{ size }
    {
    }
  };

  std::unique_ptr<imgui_props> m_props = nullptr;
};
} // namespace kogayonon_gui