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
  explicit KeyEvent( const KeyScanCode& code, const KeyScanCode& modifier, bool isPressed )
      : m_scanCode{ code }
      , m_modifier{ modifier }
  {
  }

  inline KeyScanCode getKeyScanCode() const
  {
    return m_scanCode;
  }

  inline KeyScanCode getKeyModifier() const
  {
    return m_modifier;
  }

  inline void setKeyScanCode( const KeyScanCode& code )
  {
    m_scanCode = code;
  }

  inline void setKeyModifier( const KeyScanCode& code )
  {
    m_modifier = code;
  }

protected:
  KeyScanCode m_scanCode;
  KeyScanCode m_modifier;
};

class KeyPressedEvent : public KeyEvent
{
public:
  explicit KeyPressedEvent( const KeyScanCode& keycode, const KeyScanCode& modifier, int repeatCount )
      : KeyEvent{ keycode, modifier, true }
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
  explicit KeyReleasedEvent( const KeyScanCode& keycode, const KeyScanCode& modifier )
      : KeyEvent{ keycode, modifier, false }
  {
  }
};

class KeyTypedEvent : public IEvent
{
public:
  explicit KeyTypedEvent( KeyScanCode keycode )
  {
  }
};
} // namespace kogayonon_core