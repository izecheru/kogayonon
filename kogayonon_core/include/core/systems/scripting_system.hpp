#pragma once
#include <sol/sol.hpp>

namespace kogayonon_core
{
struct MainScriptFuncs
{
  sol::protected_function init;
  sol::protected_function update;
  sol::protected_function render;
};

class ScriptingSystem
{
public:
  ScriptingSystem();
  explicit ScriptingSystem( sol::state& lua, const std::string& scriptPath );
  ~ScriptingSystem() = default;

  bool loadMainScriptFile( const std::string& path, sol::state& lua );

  static void registerBindings( sol::state& lua );

private:
  sol::state m_luaState;
  bool m_mainScriptLoaded{ false };
};
} // namespace kogayonon_core