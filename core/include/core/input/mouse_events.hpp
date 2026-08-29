#pragma once
#include "core/event/event.hpp"
#include "core/input/mouse_events.hpp"
#include "precompiled/pch.hpp"
#include "utilities/input/mouse_codes.hpp"

namespace core
{
class MouseEnteredEvent : public IEvent
{
public:
  explicit MouseEnteredEvent( const bool entered )
      : m_entered{ entered }
  {
  }

  bool hasEntered() const
  {
    return m_entered;
  }

private:
  bool m_entered;
};

class MouseMovedEvent : public IEvent
{
public:
  MouseMovedEvent( const double x, const double y, const double xRel, const double yRel )
      : m_mouseX{ x }
      , m_mouseY{ y }
      , m_xRel{ xRel }
      , m_yRel{ yRel }
  {
  }

  inline double getX() const
  {
    return m_mouseX;
  }

  inline double getY() const
  {
    return m_mouseY;
  }

  inline double getXRel() const
  {
    return m_xRel;
  }

  inline double getYRel() const
  {
    return m_yRel;
  }

private:
  double m_mouseX;
  double m_mouseY;
  double m_xRel;
  double m_yRel;
};

class MouseClickedEvent : public IEvent
{

public:
  MouseClickedEvent( int button, int action, int mods )
      : m_button{ static_cast<utilities::MouseCode>( button ) }
      , m_action{ static_cast<utilities::MouseAction>( action ) }
      , m_mods{ static_cast<utilities::MouseModifier>( mods ) }
  {
  }

  utilities::MouseCode getButton() const
  {
    return m_button;
  }

  utilities::MouseAction getAction() const
  {
    return m_action;
  }

private:
  utilities::MouseCode m_button;
  utilities::MouseAction m_action;
  utilities::MouseModifier m_mods;
};

class MouseScrolledEvent : public IEvent
{
private:
  double m_x_offset = 0;
  double m_y_offset = 0;

public:
  MouseScrolledEvent( double t_x_offset, double t_y_offset )
      : m_x_offset{ t_x_offset }
      , m_y_offset{ t_y_offset }
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
};
} // namespace core