#include "gui/imgui_windows/imgui_base.hpp"

gui::ImGuiWindow::ImGuiWindow( std::string name )
    : m_props{ std::make_unique<imgui_props>( name ) }
{
}

gui::ImGuiWindow::ImGuiWindow( std::string name, ImGuiWindowFlags flags )
    : m_props{ std::make_unique<imgui_props>( name, flags ) }
{
}

gui::ImGuiWindow::ImGuiWindow( std::string name, ImGuiWindowFlags flags, ImVec2 size )
    : m_props{ std::make_unique<imgui_props>( name, flags, size ) }

{
  ImGui::SetWindowSize( size );
}

gui::ImGuiWindow::ImGuiWindow( std::string name, ImVec2 size )
    : m_props{ std::make_unique<imgui_props>( name, size ) }
{
}

std::string gui::ImGuiWindow::getName() const
{
  return m_props->name;
}

void gui::ImGuiWindow::hide()
{
  m_props->visible = false;
}

void gui::ImGuiWindow::show()
{
  m_props->visible = true;
}

void gui::ImGuiWindow::updatePosition()
{
  auto pos = ImGui::GetWindowPos();
  if ( m_props->x == pos.x && m_props->y == pos.y )
    return;

  m_props->x = pos.x;
  m_props->y = pos.y;
}

void gui::ImGuiWindow::updateSize()
{
  auto size = ImGui::GetWindowSize();
  if ( m_props->width == size.x && m_props->height == size.y )
    return;

  m_props->height = static_cast<int>( size.y );
  m_props->width = static_cast<int>( size.x );
}

void gui::ImGuiWindow::setBounds()
{
  // auto max = ImGui::GetWindowContentRegionMax();
  // auto min = ImGui::GetWindowContentRegionMin();
  // m_props->bounds.bottomRight = ImVec2{ max.x + m_props->x, max.y + m_props->y };
  // m_props->bounds.topLeft = ImVec2{ min.x + m_props->x, min.y + m_props->y };
}

void gui::ImGuiWindow::updateFocused()
{
  m_props->focused = ImGui::IsWindowFocused();
}

void gui::ImGuiWindow::updateHovered()
{
  m_props->hovered = ImGui::IsWindowHovered();
}

void gui::ImGuiWindow::setDocked()
{
  m_props->hovered = ImGui::IsWindowDocked();
}

void gui::ImGuiWindow::initProps()
{
  updatePosition();
  updateSize();
  updateFocused();
  updateHovered();
}

bool gui::ImGuiWindow::begin()
{
  if ( !m_props->visible )
    return false;

  if ( !ImGui::Begin( m_props->name.c_str(), &m_props->visible, m_props->flags ) )
  {
    end();
    return false;
  }

  initProps();
  return true;
}

void gui::ImGuiWindow::end()
{
  ImGui::End();
}