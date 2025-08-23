#pragma once
#include <sstream>
#include <unordered_map>

#include "event/event.h"
#include "input/keyboard_state.h"
#include "key_codes.h"

namespace kogayonon
{
  class KeyEvent : public Event
  {
  public:
    KeyEvent(const KeyCode& code) : m_key_code(code) {}

    KeyCode getKeyCode() const
    {
      return m_key_code;
    }

  protected:
    KeyCode m_key_code;
  };

  class KeyPressedEvent : public Event
  {
  public:
    KeyPressedEvent(KeyCode&& keycode, int repeatCount) : m_RepeatCount(repeatCount)
    {
      KeyboardState::setKeyState(std::move(keycode), true);
    }

    inline int GetRepeatCount() const
    {
      return m_RepeatCount;
    }

    EVENT_CLASS_TYPE(KeyTyped)

  private:
    int m_RepeatCount;
  };

  class KeyReleasedEvent : public Event
  {
  public:
    KeyReleasedEvent(KeyCode&& keycode)
    {
      KeyboardState::setKeyState(keycode, false);
    }

    EVENT_CLASS_TYPE(KeyReleased)
  };

  class KeyTypedEvent : public Event
  {
  public:
    KeyTypedEvent(KeyCode keycode) {}
    EVENT_CLASS_TYPE(KeyTyped)
  };
} // namespace kogayonon
