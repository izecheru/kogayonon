#pragma once
#include <sstream>

#include "event.h"

namespace kogayonon
{
  class WindowCloseEvent : public Event
  {
  public:
    WindowCloseEvent() = default;
    EVENT_CLASS_TYPE(WindowClose)
  };

  class WindowResizeEvent : public Event
  {
  public:
    WindowResizeEvent(unsigned int width, unsigned int height) : m_Width(width), m_Height(height) {}

    unsigned int getWidth() const
    {
      return m_Width;
    }

    unsigned int getHeight() const
    {
      return m_Height;
    }

    EVENT_CLASS_TYPE(WindowResize)

  private:
    unsigned int m_Width, m_Height;
  };
} // namespace kogayonon
