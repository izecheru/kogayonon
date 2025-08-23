#pragma once
#include <unordered_map>

#include "key_codes.h"

namespace kogayonon
{
  class KeyboardState
  {
  public:
    KeyboardState() = delete;
    ~KeyboardState() = default;

    static void setKeyState(KeyCode code, bool state)
    {
      if (key_state.find(code) != key_state.end())
      {
        key_state.at(code) = state;
      }
    }

    static bool getKeyState(KeyCode code)
    {
      return key_state[code];
    }

    static bool getKeyCombinationState(std::vector<KeyCode>& codes)
    {
      bool result = true;
      for (KeyCode& code : codes)
      {
        // if all are true &=
        result &= key_state.at(code);
      }
      return result;
    }

  public:
    static inline std::unordered_map<KeyCode, bool> key_state;
  };
} // namespace kogayonon