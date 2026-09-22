#pragma once
#include <SDL3/SDL_keyboard.h>
#include "key_codes.hpp"
#include "precompiled/pch.hpp"

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
        return m_keyboardState[static_cast<int>( code )] != 0;
    }

    static inline bool getKeyCombinationState( const std::vector<KeyScanCode>& codes )
    {
        bool result = true;
        for ( auto& code : codes )
        {
            result &= ( m_keyboardState[static_cast<int>( code )] != 0 );
        }
        return result;
    }

  private:
    static inline const bool* m_keyboardState{ nullptr };
};
} // namespace utilities