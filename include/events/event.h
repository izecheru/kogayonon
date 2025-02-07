#pragma once
#include <string>

namespace kogayonon
{


#define BIT(x) (1<<x)

  enum class EventType
  {
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMinimzed,
    AppTick, AppUpdate, AppRender,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled, MouseEntered
  };

  enum EventCategory
  {
    None = 0,
    ApplicationEventCategory = BIT(0),
    InputEventCategory = BIT(1),
    KeyboardEventCategory = BIT(2),
    MouseEventCategory = BIT(3),
    MouseButtonEventCategory = BIT(4)
  };


#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override { return category; }


#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::type; }\
								virtual EventType getEventType() const override { return getStaticType(); }\
								virtual const char* getName() const override { return #type; }


  class Event
  {
  public:
    virtual ~Event() = default;
    bool Handled = false;

    virtual EventType getEventType() const = 0;
    virtual const char* getName() const = 0;
    virtual int getCategoryFlags() const = 0;
    virtual std::string toString() const = 0;
    bool isInCategory(EventCategory category);
  };

  class EventDispatcher
  {
  private:
    Event& m_event;

  public:
    EventDispatcher(Event& event) : m_event(event) {}

    template<typename T, typename F>
    bool dispatch(const F& func) {
      if (m_event.getEventType() == T::getStaticType())
      {
        m_event.Handled |= func(static_cast<T&>(m_event));
        return true;
      }
      return false;
    }
  };
}
