#pragma once

#include "core/event/event.hpp"
#include "utilities/input/key_codes.hpp"
#include "utilities/input/keyboard_state.hpp"
using namespace kogayonon_utilities;

namespace kogayonon_core
{
class KeyEvent : public IEvent
{
public:
  explicit KeyEvent( const KeyCode& code, bool isPressed )
      : m_keyCode{ code }
  {
    if ( isPressed )
    {
      KeyboardState::setKeyState( code, true );
    }
    else
    {
      KeyboardState::setKeyState( code, false );
    }
  }

  KeyCode getKeyCode() const
  {
    return m_keyCode;
  }

protected:
  KeyCode m_keyCode;
};

class KeyPressedEvent : public KeyEvent
{
public:
  explicit KeyPressedEvent( KeyCode keycode, int repeatCount )
      : KeyEvent{ keycode, true }
      , m_RepeatCount{ repeatCount }
  {
  }

  inline int GetRepeatCount() const
  {
    return m_RepeatCount;
  }

private:
  int m_RepeatCount;
};

class KeyReleasedEvent : public KeyEvent
{
public:
  explicit KeyReleasedEvent( KeyCode keycode )
      : KeyEvent{ keycode, false }
  {
  }
};

class KeyTypedEvent : public IEvent
{
public:
  explicit KeyTypedEvent( KeyCode keycode )
  {
  }
};
} // namespace kogayonon_core