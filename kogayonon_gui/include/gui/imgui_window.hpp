#pragma once
#include <functional>
#include <imgui.h>
#include <memory>
#include <string>

namespace kogayonon_gui
{
class ImGuiWindow
{
    using RenderCallbackFn = std::function<void()>;

  public:
    ImGuiWindow(std::string name);
    virtual ~ImGuiWindow();
    virtual void draw() = 0;

    std::string getName() const;

    virtual void setDocked(bool status);
    virtual void setVisible(bool status);
    virtual void setCallback(const RenderCallbackFn& func);
    virtual void setX(double x);
    virtual void setY(double y);

    int width();
    int height();

  protected:

    // if we ever need to pass a rendering func in the middle of the window or something
    // i made this for the scene viewport so i can pass functions from Renderer class
    RenderCallbackFn m_renderCallback;

    struct imgui_props
    {
        std::string name;
        double x = 0.0;
        double y = 0.0;
        int width = 0;
        int height = 0;
        bool docked = false;
        bool canMove = true;
        bool visible = true;
        bool hovered = false;
        bool resizable = false;
        ImGuiWindowFlags flags;

        explicit imgui_props(std::string t_name, std::vector<std::string> t_fonts = {})
            : name(std::move(t_name)), x(0), y(0), docked(false), visible(true), hovered(false), canMove(true),
              resizable(false), flags(0)
        {
        }
    };

    std::unique_ptr<imgui_props> m_props = nullptr;
};
} // namespace kogayonon_gui
