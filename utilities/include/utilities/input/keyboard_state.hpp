#pragma once
#include "key_codes.hpp"
#include "precompiled/pch.hpp"
#include <SDL2/SDL.h>

namespace utilities
{
class KeyboardState
{
public:
  KeyboardState() = delete;
  ~KeyboardState() = default;

  static inline void updateState()
  {
    SDL_PumpEvents();
  }

  static inline void initState()
  {
    m_keyboardState = SDL_GetKeyboardState( NULL );
  }

  static inline bool getKeyState( const KeyScanCode& code )
  {
    return m_keyboardState[static_cast<int>( code )];
  }

  static inline bool getKeyCombinationState( const std::vector<KeyScanCode>& codes )
  {
    bool result = true;
    for ( auto& code : codes )
    {
      // if all are true &=
      result &= static_cast<bool>( m_keyboardState[static_cast<int>( code )] );
    }
    return result;
  }

private:
  static inline const Uint8* m_keyboardState;
};
} // namespace utilities