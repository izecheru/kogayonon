#pragma once
#include "../event.h"

namespace kogayonon {

  class MouseMoved : public Event {
  private:
    float m_MouseX, m_MouseY;

  public:
    MouseMoved(float x, float y) : m_MouseX(x), m_MouseY(y) {}

    float GetX();
    float GetY();

    std::string toString();

    static EventType getStaticType() {
      return EventType::MouseMoved;
    }

    virtual EventType getEventType() const override {
      return getStaticType();
    }

    virtual const char* getName() const override {
      return "MouseMoved";
    };

    virtual int getCategoryFlags() const override {
      return MouseEventCategory | InputEventCategory;
    };
  };

}
