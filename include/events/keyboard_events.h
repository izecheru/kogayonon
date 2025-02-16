#pragma once
#include <sstream>
#include <unordered_map>

#include "core/key_codes.h"
#include "events/event.h"
namespace kogayonon
{
  class KeyboardState
  {
  public:
    KeyboardState() = delete;
    ~KeyboardState() = default;

    static void setKeyState(KeyCode code, bool state) { key_state[code] = state; }
    static bool getKeyState(KeyCode code) { return key_state[code]; }
    static bool getKeyCombinationState(std::vector<KeyCode> codes) {
      bool result = true;
      for (KeyCode& code : codes) {
        // if all are true &=
        result &= key_state[code];
      }
      return result;
    }

  public:
    static inline std::unordered_map<KeyCode, bool> key_state;
  };

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
      : KeyEvent(keycode), m_RepeatCount(repeatCount) {
      KeyboardState::setKeyState(keycode, true);
    }

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
      : KeyEvent(keycode) {
      KeyboardState::setKeyState(keycode, false);
    }

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
}
