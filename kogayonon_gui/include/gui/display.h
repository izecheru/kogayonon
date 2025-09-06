#pragma once
#include "imgui_window.h"

namespace kogayonon_gui {
/**
 * @brief Base class for implementing menus and easier drawing in custom imgui windows
 */
class Display
{
  public:
    Display(std::string name) : m_name(std::move(name)) {}

    virtual ~Display() {}

    inline std::string getName() const
    {
        return m_name;
    }

    // does not need to have ImGui::Begin since it always resides in an already running window
    virtual void draw() = 0;

  private:
    std::string m_name;
};
} // namespace kogayonon_gui