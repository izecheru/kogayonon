#pragma once
#include <sol/sol.hpp>

namespace kogayonon_core
{
class ScriptingSystem
{
public:
  ScriptingSystem();
  explicit ScriptingSystem( sol::state& lua, const std::string& scriptPath );
  ~ScriptingSystem() = default;

  bool loadMainScriptFile( const std::string& path, sol::state& lua );

  static void registerBindings( sol::state& lua );

private:
  bool m_mainScriptLoaded{ false };
  sol::state m_luaState;
};
} // namespace kogayonon_core