#include "gui/imgui_window.h"

namespace kogayonon_gui
{
ImGuiWindow::ImGuiWindow(std::string name)
{
    m_props = std::make_unique<imgui_props>(name);
}

ImGuiWindow::~ImGuiWindow()
{
}

std::string ImGuiWindow::getName() const
{
    return m_props->name;
}

void ImGuiWindow::setDocked(bool status)
{
    m_props->docked = status;
}

void ImGuiWindow::setVisible(bool status)
{
    m_props->visible = status;
}

void ImGuiWindow::setX(double x)
{
    m_props->x = x;
}

void ImGuiWindow::setY(double y)
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

void ImGuiWindow::setCallback(const RenderCallbackFn& func)
{
    m_renderCallback = func;
}
} // namespace kogayonon_gui