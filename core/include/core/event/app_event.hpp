#pragma once
#include "event.hpp"

namespace core
{
class WindowCloseEvent : public IEvent
{
public:
  WindowCloseEvent() = default;

private:
};

class WindowResizeEvent : public IEvent

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

private:
  int m_Width, m_Height;
};
} // namespace core