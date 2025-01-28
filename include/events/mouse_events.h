#pragma once
#include <sstream>
#include <events/event.h>
#include <core/mouse_codes.h>
#include <string>


class MouseEnteredEvent :public Event
{
public:
  MouseEnteredEvent(const int entered) :m_entered(entered) {}

  int hasEntered() const { return m_entered; }

  std::string toString()const override {
    std::stringstream ss{};
    ss << "MouseEnteredEvent: " << m_entered;
    std::string result = ss.str();
    return result;
  }

  EVENT_CLASS_CATEGORY(MouseEventCategory)
    EVENT_CLASS_TYPE(MouseEntered)
private:
  int m_entered;
};

class MouseMovedEvent : public Event
{
private:
  double m_mouse_x, m_mouse_y;

public:
  MouseMovedEvent(const double x, const double y)
    : m_mouse_x(x)
    , m_mouse_y(y) {}

  double getX() { return m_mouse_x; }
  double getY() { return m_mouse_y; }

  std::string toString()const override {
    std::stringstream ss{};
    ss << "MouseMovedEvent: " << m_mouse_x << ", " << m_mouse_y;
    std::string result = ss.str();
    return result;
  }

  EVENT_CLASS_CATEGORY(MouseEventCategory)
    EVENT_CLASS_TYPE(MouseMoved)
};

class MouseClickedEvent :public Event
{
private:
  //WARNING maybe i will somehow need the coordinates as well
  MouseCode m_button;
  MouseAction m_action;
  MouseModifier m_mods;

public:
  MouseClickedEvent(const MouseCode button, const MouseAction action, const  MouseModifier mods) :m_button(button), m_action(action), m_mods(mods) {}

  MouseCode getButton() const { return m_button; }
  MouseAction getAction() const { return m_action; }

  std::string toString() const override {
    std::stringstream ss{};
    ss << "MouseClickedEvent: " << m_button << " " << m_action;
    std::string result = ss.str();
    return result;
  }

  EVENT_CLASS_CATEGORY(MouseEventCategory)
    EVENT_CLASS_TYPE(MouseButtonPressed)
};

