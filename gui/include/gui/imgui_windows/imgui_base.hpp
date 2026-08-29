#pragma once
#include "precompiled/pch.hpp"
#include <cstdint>
#include <imgui.h>

namespace gui
{
struct ImGuiProps
{
  std::string name;
  double x{ 0.0 };
  double y{ 0.0 };
  uint32_t width{ 0u };
  uint32_t height{ 0u };
  bool canMove{ true };
  bool visible{ true };
  bool docked{ false };
  bool hovered{ false };
  bool resizable{ false };
  bool focused{ false };
  ImGuiWindowFlags flags{ 0 };
  ImVec2 size{ 0.0f, 0.0f };

  explicit ImGuiProps( std::string t_name )
      : name{ std::move( t_name ) }
  {
  }

  explicit ImGuiProps( std::string t_name, ImGuiWindowFlags t_flags )
      : name{ t_name }
      , flags{ t_flags }
  {
  }

  explicit ImGuiProps( std::string t_name, ImGuiWindowFlags t_flags, ImVec2 size )
      : name{ t_name }
      , flags{ t_flags }
      , size{ size }
  {
  }

  explicit ImGuiProps( std::string t_name, ImVec2 size )
      : name{ t_name }
      , size{ size }
  {
  }
};

class ImGuiWindow
{
public:
  explicit ImGuiWindow( std::string name );
  explicit ImGuiWindow( std::string name, ImGuiWindowFlags flags );
  explicit ImGuiWindow( std::string name, ImGuiWindowFlags flags, ImVec2 size );
  explicit ImGuiWindow( std::string name, ImVec2 size );
  virtual ~ImGuiWindow() = default;

  virtual void render();

  auto getName() const -> std::string;
  auto getProps() -> ImGuiProps*;

  void hide();
  void show();

  virtual void setDocked();
  virtual void updateHovered();
  virtual void updateFocused();

  virtual void updatePosition();
  virtual void updateSize();

  virtual bool begin();
  virtual void end();

  /**
   * @brief Sets up width, height, x, y when called
   */
  virtual void updateProps();

protected:
  std::unique_ptr<ImGuiProps> m_props;
};
} // namespace gui
