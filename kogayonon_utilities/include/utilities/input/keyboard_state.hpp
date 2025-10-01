#pragma once
#include <unordered_map>

#include "key_codes.hpp"

namespace kogayonon_utilities
{
class KeyboardState
{
public:
  KeyboardState() = delete;
  ~KeyboardState() = default;

  static void setKeyState( KeyCode code, bool state )
  {
    if ( keyState.contains( code ) )
    {
      keyState.at( code ) = state;
    }
  }

  static bool getKeyState( KeyCode code )
  {
    return keyState[code];
  }

  static bool getKeyCombinationState( const std::vector<KeyCode>& codes )
  {
    bool result = true;
    for ( auto& code : codes )
    {
      // if all are true &=
      result &= keyState.at( code );
    }
    return result;
  }

public:
  static inline std::unordered_map<KeyCode, bool> keyState;
};
} // namespace kogayonon_utilities