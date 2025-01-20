#pragma once
#include "event.h"
#include <sstream>

namespace kogayonon
{
  class WindowResizeEvent : public Event
  {
  public:
    WindowResizeEvent(unsigned int width, unsigned int height)
      : m_Width(width), m_Height(height)
    {}

    unsigned int getWidth() const
    {
      return m_Width;
    }
    unsigned int getHeight() const
    {
      return m_Height;
    }

    std::string toString() const override
    {
      std::stringstream ss;
      ss << "WindowResizeEvent: " << m_Width << ", " << m_Height;
      return ss.str();
    }

    EVENT_CLASS_TYPE(WindowResize)

      EVENT_CLASS_CATEGORY(ApplicationEventCategory)
  private:
    unsigned int m_Width, m_Height;
  };
};
