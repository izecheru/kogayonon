#pragma once
#include <sstream>

#include "event.hpp"

namespace kogayonon_core
{
class WindowCloseEvent : public Event
{
public:
  WindowCloseEvent() = default;
  EVENT_CLASS_TYPE( WindowClose )
};

class WindowResizeEvent : public Event
{
public:
  WindowResizeEvent( int width, int height )
      : m_Width( width )
      , m_Height( height )
  {
  }

  int getWidth() const
  {
    return m_Width;
  }

  int getHeight() const
  {
    return m_Height;
  }

  EVENT_CLASS_TYPE( WindowResize )

private:
  int m_Width, m_Height;
};
} // namespace kogayonon_core