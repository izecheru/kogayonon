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
  ~ScriptingSystem() = default;

  /**
   * @brief Registers the lua bindings from all the defined usertypes
   * @param lua Lua state reference variable
   */
  static void registerBindings( sol::state& lua );

  bool isInit() const;

  void loadMainScript( const std::string& path );

  auto getLuaState() -> sol::state&;

private:
  sol::state m_luaState;
  bool m_init;
};
} // namespace kogayonon_core