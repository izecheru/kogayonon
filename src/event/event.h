#pragma once
#include <string>

namespace kogayonon
{
  enum class EventType
  {
    None = 0,
    WindowClose,
    WindowResize,
    WindowFocus,
    WindowLostFocus,
    WindowMinimzed,
    AppTick,
    AppUpdate,
    AppRender,
    KeyPressed,
    KeyReleased,
    KeyTyped,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseMoved,
    MouseScrolled,
    MouseEntered,
    MouseClicked
  };

  // clang-format off
#define EVENT_CLASS_TYPE(type) static EventType getStaticType() { return EventType::type; }\
								virtual EventType getEventType() const override { return getStaticType(); }

  // clang-format on

  class Event
  {
  public:
    Event() = default;
    virtual ~Event() = default;

    virtual EventType getEventType() const = 0;

    inline bool isHandled() const
    {
      return m_handled;
    }

    inline void setHandled()
    {
      m_handled = true;
    }

    bool m_handled = false;
  };
} // namespace kogayonon
