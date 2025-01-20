#pragma once
#include <glfw3.h>
#include <string>
#include <functional>
namespace kogayonon
{

#define BIT(x) (1<<x)
#define HZ_BIND_EVENT_FN3(fn) \
[](auto&&... args) { \
    std::cout << "Binding function " << #fn << std::endl; \
    return fn(std::forward<decltype(args)>(args)...); \
}
#define HZ_BIND_EVENT_FN2(fn) std::bind(&fn, this, std::placeholders::_1)
#define HZ_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }
  enum class EventType
  {
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMinimzed,
    AppTick, AppUpdate, AppRender,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
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

#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::type; }\
								virtual EventType getEventType() const override { return getStaticType(); }\
								virtual const char* getName() const override { return #type; }

#define EVENT_CLASS_CATEGORY(category) virtual int getCategoryFlags() const override { return category; }

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
    template<typename T>
    using EventFn = std::function<void(T&)>;
  private:
    Event& m_event;

  public:
    EventDispatcher(Event& event) : m_event(event)
    {}

    template<typename T, typename F>
    bool Dispatch(const F& func)
    {
      if (m_event.getEventType() == T::getStaticType())
      {
        bool result = func(static_cast<T&>(m_event));
        m_event.Handled |= result;
        return true;
      }
      return false;
    }
  };
}
