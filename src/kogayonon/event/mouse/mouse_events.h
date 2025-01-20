#pragma once
#include "../event.h"

namespace kogayonon
{

  class MouseMovedEvent : public Event
  {
  private:
    float m_MouseX, m_MouseY;

  public:
    MouseMovedEvent(float x, float y)
      : m_MouseX(x)
      , m_MouseY(y)
    {}

    float getX();
    float getY();

    std::string toString()const override;

    EVENT_CLASS_CATEGORY(MouseEventCategory)
      EVENT_CLASS_TYPE(MouseMoved)
  };

  class MouseClickedEvent :public Event
  {
  private:
    float m_xpos, m_ypos;

  public:
    MouseClickedEvent(float x_p, float y_p) :m_xpos(x_p), m_ypos(y_p) {}

    float getX();
    float getY();

    std::string toString() const override;

    EVENT_CLASS_CATEGORY(MouseEventCategory)
      EVENT_CLASS_TYPE(MouseButtonPressed)
  };

} // namespace kogayonon
