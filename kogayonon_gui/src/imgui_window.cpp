#include "gui/imgui_window.hpp"

namespace kogayonon_gui
{
ImGuiWindow::ImGuiWindow( std::string name )
    : m_props{ std::make_unique<imgui_props>( name ) }
{
}

ImGuiWindow::ImGuiWindow( std::string name, ImGuiWindowFlags flags )
    : m_props{ std::make_unique<imgui_props>( name, flags ) }
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
    m_props->height = static_cast<int>( size.x );
    m_props->width = static_cast<int>( size.y );

    auto max = ImGui::GetWindowContentRegionMax();
    auto min = ImGui::GetWindowContentRegionMin();
    m_props->bounds.bottomRight = ImVec2{ max.x + pos.x, max.y + pos.y };
    m_props->bounds.topLeft = ImVec2{ min.x + pos.x, min.y + pos.y };
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

  if ( m_props->bounds.bottomRight.x == 0.0f )
  {
    auto pos = ImGui::GetWindowPos();
    auto max = ImGui::GetWindowContentRegionMax();
    auto min = ImGui::GetWindowContentRegionMax();
    m_props->bounds.bottomRight = ImVec2{ max.x + pos.x, max.y + pos.y };
    m_props->bounds.topLeft = ImVec2{ min.x + pos.x, min.y + pos.y };
  }
}

void ImGuiWindow::begin()
{
  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    end();
    return;
  }
}

void ImGuiWindow::end()
{
  ImGui::End();
}
} // namespace kogayonon_gui