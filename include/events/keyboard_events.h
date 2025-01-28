#pragma once
#include <sstream>

#include "core/key_codes.h"
#include "events/event.h"

class KeyEvent :public Event
{
public:
  KeyEvent(const KeyCode code) :m_key_code(code) {}
  KeyCode getKeyCode() const { return m_key_code; }

  EVENT_CLASS_CATEGORY(KeyboardEventCategory | InputEventCategory)
protected:
  KeyCode m_key_code;
};

class KeyPressedEvent : public KeyEvent
{
public:
  KeyPressedEvent(KeyCode keycode, int repeatCount)
    : KeyEvent(keycode), m_RepeatCount(repeatCount) {}

  inline int GetRepeatCount() const { return m_RepeatCount; }

  std::string toString() const override {
    std::stringstream ss{};
    ss << "KeyPressedEvent: " << m_key_code << " (" << m_RepeatCount << " repeats)";
    std::string result = ss.str();
    return result;
  }

  EVENT_CLASS_TYPE(KeyPressed)
private:
  int m_RepeatCount;
};

class KeyReleasedEvent : public KeyEvent
{
public:
  KeyReleasedEvent(KeyCode keycode)
    : KeyEvent(keycode) {}

  std::string toString() const override {
    std::stringstream ss;
    ss << "KeyReleasedEvent: " << static_cast<char>(m_key_code);
    std::string result = ss.str();
    return result;
  }

  EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent : public KeyEvent
{
public:
  KeyTypedEvent(KeyCode keycode)
    : KeyEvent(keycode) {}

  std::string toString() const override {
    std::stringstream ss;
    ss << "KeyTypedEvent: " << static_cast<char>(m_key_code);
    std::string result = ss.str();
    return ss.str();
  }

  EVENT_CLASS_TYPE(KeyTyped)
};

