#pragma once
#include <SDL3/SDL_mouse.h>
#include "glm/glm.hpp"

namespace utilities
{
class MouseState
{
  public:
    MouseState() = delete;
    ~MouseState() = default;

    static auto getRel() -> glm::vec2
    {
        return m_relAccumulated;
    }

    static auto resetRel() -> void
    {
        m_relAccumulated = { 0.0f, 0.0f };
    }

    static auto updateRel( glm::vec2 rel ) -> void
    {
        m_relAccumulated += rel;
    }

  private:
    static inline glm::vec2 m_mouseCoord;
    static inline glm::vec2 m_rel;
    static inline glm::vec2 m_relAccumulated;
};
} // namespace utilities
