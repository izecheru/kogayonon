#pragma once

#include "core/event/event.hpp"
#include "core/input/key_codes.hpp"
#include "core/input/keyboard_state.hpp"

namespace kogayonon_core
{
class KeyEvent : public IEvent
{
public:
  KeyEvent( const KeyCode& code, bool isPressed )
      : m_keyCode( code )
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
  KeyPressedEvent( KeyCode keycode, int repeatCount )
      : KeyEvent( keycode, true )
      , m_RepeatCount( repeatCount )
  {
  }

  inline int GetRepeatCount() const
  {
    return m_RepeatCount;
  }

  EVENT_CLASS_TYPE( KeyPressed )

private:
  int m_RepeatCount;
};

class KeyReleasedEvent : public KeyEvent
{
public:
  KeyReleasedEvent( KeyCode keycode )
      : KeyEvent( keycode, false )
  {
  }

  EVENT_CLASS_TYPE( KeyReleased )
};

class KeyTypedEvent : public IEvent
{
public:
  KeyTypedEvent( KeyCode keycode )
  {
  }
  EVENT_CLASS_TYPE( KeyTyped )
};
} // namespace kogayonon_core