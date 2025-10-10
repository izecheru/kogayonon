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

std::string ImGuiWindow::getName() const
{
  return m_props->name;
}

void ImGuiWindow::setPosition()
{
  auto pos = ImGui::GetWindowPos();
  if ( m_props->x == pos.x && m_props->y == pos.y )
    return;

  m_props->x = pos.x;
  m_props->y = pos.y;
}

void ImGuiWindow::setSize()
{
  auto size = ImGui::GetWindowSize();
  if ( m_props->width == size.x && m_props->height == size.y )
    return;

  m_props->height = static_cast<int>( size.y );
  m_props->width = static_cast<int>( size.x );
}

void ImGuiWindow::setBounds()
{
  // auto max = ImGui::GetWindowContentRegionMax();
  // auto min = ImGui::GetWindowContentRegionMin();
  // m_props->bounds.bottomRight = ImVec2{ max.x + m_props->x, max.y + m_props->y };
  // m_props->bounds.topLeft = ImVec2{ min.x + m_props->x, min.y + m_props->y };
}

void ImGuiWindow::setFocused()
{
  m_props->focused = ImGui::IsWindowFocused();
}

void ImGuiWindow::setHovered()
{
  m_props->hovered = ImGui::IsWindowHovered();
}

void ImGuiWindow::setDocked()
{
  m_props->hovered = ImGui::IsWindowDocked();
}

void ImGuiWindow::initProps()
{
  setPosition();
  setSize();
  setFocused();
  setHovered();

  // setBounds();
}

bool ImGuiWindow::begin()
{
  if ( !m_props->visible )
    return false;

  ImGui::Begin( m_props->name.c_str() );
  return true;
}

void ImGuiWindow::end()
{
  ImGui::End();
}
} // namespace kogayonon_gui