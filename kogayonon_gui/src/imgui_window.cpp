#include "gui/imgui_window.hpp"

namespace kogayonon_gui
{
ImGuiWindow::ImGuiWindow( std::string name )
    : m_props{ std::make_unique<imgui_props>( name ) }
{
}

ImGuiWindow::~ImGuiWindow()
{
}

std::string ImGuiWindow::getName() const
{
  return m_props->name;
}

void ImGuiWindow::setDocked( bool status )
{
  m_props->docked = status;
}

void ImGuiWindow::setVisible( bool status )
{
  m_props->visible = status;
}

void ImGuiWindow::setX( double x )
{
  m_props->x = x;
}

void ImGuiWindow::setY( double y )
{
  m_props->y = y;
}

int ImGuiWindow::width()
{
  return m_props->width;
}

int ImGuiWindow::height()
{
  return m_props->height;
}

void ImGuiWindow::setupProportions( bool dirty )
{
  // if dirty set them regardless
  if ( dirty )
  {
    auto pos = ImGui::GetWindowPos();
    m_props->x = pos.x;
    m_props->y = pos.y;

    auto size = ImGui::GetWindowSize();
    m_props->height = static_cast<int>( size.y );
    m_props->width = static_cast<int>( size.x );

    return;
  }

  if ( m_props->x == 0 && m_props->y == 0 )
  {
    auto pos = ImGui::GetWindowPos();
    m_props->x = pos.x;
    m_props->y = pos.y;
  }

  if ( m_props->height == 0 && m_props->width == 0 )
  {
    auto size = ImGui::GetWindowSize();
    m_props->height = static_cast<int>( size.y );
    m_props->width = static_cast<int>( size.x );
  }
}
} // namespace kogayonon_gui