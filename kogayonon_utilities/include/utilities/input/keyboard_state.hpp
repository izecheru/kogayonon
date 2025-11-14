#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include "key_codes.hpp"

namespace kogayonon_utilities
{
class KeyboardState
{
public:
  KeyboardState() = delete;
  ~KeyboardState() = default;

  static void updateState()
  {
    SDL_PumpEvents();
  }

  static void initState()
  {
    m_keyboardState = SDL_GetKeyboardState( NULL );
  }

  static bool getKeyState( KeyScanCode code )
  {
    return m_keyboardState[code];
  }

  static bool getKeyCombinationState( const std::vector<KeyScanCode>& codes )
  {
    bool result = true;
    for ( auto& code : codes )
    {
      // if all are true &=
      result &= m_keyboardState[code];
    }
    return result;
  }

private:
  static inline const Uint8* m_keyboardState;
};
} // namespace kogayonon_utilities