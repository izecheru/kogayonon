#pragma once
#include <sstream>
#include <string>

#include "core/event/event.hpp"
#include "core/input/mouse_codes.hpp"
#include "core/input/mouse_events.hpp"

namespace kogayonon_core
{
class MouseEnteredEvent : public IEvent
{
public:
  explicit MouseEnteredEvent( const int entered )
      : m_entered( entered )
  {
  }

  int hasEntered() const
  {
    return m_entered;
  }

  EVENT_CLASS_TYPE( MouseEntered )

private:
  int m_entered;
};

class MouseMovedEvent : public IEvent
{
private:
  double m_mouse_x;
  double m_mouse_y;

public:
  MouseMovedEvent( const double x, const double y, const double xRel, const double yRel )
      : m_mouse_x{ x }
      , m_mouse_y{ y }
      , m_xRel{ xRel }
      , m_yRel{ yRel }
  {
  }

  inline double getX() const
  {
    return m_mouse_x;
  }

  inline double getY() const
  {
    return m_mouse_y;
  }

  inline double getXRel() const
  {
    return m_xRel;
  }

  inline double getYRel() const
  {
    return m_yRel;
  }

  EVENT_CLASS_TYPE( MouseMoved )

private:
  double m_xRel;
  double m_yRel;
};

class MouseClickedEvent : public IEvent
{
private:
  MouseCode m_button;
  MouseAction m_action;
  MouseModifier m_mods;

public:
  MouseClickedEvent( int button, int action, int mods )
      : m_button( MouseCode( button ) )
      , m_action( MouseAction( action ) )
      , m_mods( MouseModifier( mods ) )
  {
  }

  MouseCode getButton() const
  {
    return m_button;
  }

  MouseAction getAction() const
  {
    return m_action;
  }
  EVENT_CLASS_TYPE( MouseClicked )
};

class MouseScrolledEvent : public IEvent
{
private:
  double m_x_offset = 0;
  double m_y_offset = 0;

public:
  MouseScrolledEvent( double t_x_offset, double t_y_offset )
      : m_x_offset( t_x_offset )
      , m_y_offset( t_y_offset )
  {
  }

  inline double getXOff() const
  {
    return m_x_offset;
  }

  inline double getYOff() const
  {
    return m_y_offset;
  }
  EVENT_CLASS_TYPE( MouseScrolled )
};
} // namespace kogayonon_core