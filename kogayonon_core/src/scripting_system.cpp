#include "core/systems/scripting_system.hpp"
#include <filesystem>
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/identifier_component.hpp"
#include "core/ecs/components/index_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/outline_component.hpp"
#include "core/ecs/components/pointlight_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/ecs/registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "utilities/time_tracker/time_tracker.hpp"

using namespace kogayonon_utilities;

namespace kogayonon_core
{
ScriptingSystem::ScriptingSystem()
    : m_luaState{ sol::state{} }
{
  auto absPath = std::filesystem::absolute( "." );
  m_luaState.open_libraries( sol::lib::base, sol::lib::package, sol::lib::string );

  absPath = absPath / "resources\\scripts\\main.lua";
  m_mainScriptLoaded = loadMainScriptFile( absPath.string(), m_luaState );

  if ( m_mainScriptLoaded )
  {
    spdlog::error( "could not load the main script file" );
  }
}

ScriptingSystem::ScriptingSystem( sol::state& lua, const std::string& scriptPath )
{
  m_mainScriptLoaded = loadMainScriptFile( scriptPath, lua );

  if ( m_mainScriptLoaded )
  {
    spdlog::error( "could not load the main script file" );
  }
}

bool ScriptingSystem::loadMainScriptFile( const std::string& path, sol::state& lua )
{
  if ( !std::filesystem::exists( path ) )
  {
    spdlog::error( "File does not exist:{}", path );
    return false;
  }

  registerBindings( lua );

  // get the main script ptr from the main registry
  auto mainScript = std::make_shared<MainScriptFuncs>();

  // load the lua script file
  lua.safe_script_file( path );

  // check for update and render funcs
  sol::optional<sol::table> mainLua = lua["main"];
  if ( mainLua )
  {
    sol::optional<sol::function> update = ( *mainLua )["update"];
    sol::optional<sol::function> render = ( *mainLua )["render"];
    if ( update && render )
    {
      mainScript->update = *update;
      mainScript->render = *render;

      // test call
      ( *render )();
      ( *update )();

      MainRegistry::getInstance().addToContext<std::shared_ptr<MainScriptFuncs>>( std::move( mainScript ) );
      return true;
    }
    else
    {
      spdlog::error( "Main is missing from the script file" );
    }
  }
  return false;
}

void ScriptingSystem::registerBindings( sol::state& lua )
{
  Entity::createLuaBindings( lua );
  Registry::createLuaBindings( lua );
  TimeTracker::createLuaBindings( lua );
  EventDispatcher::createLuaBindings( lua );

  registerMetaComponent<DirectionalLightComponent>();
  registerMetaComponent<DynamicRigidbodyComponent>();
  registerMetaComponent<StaticRigidbodyComponent>();
  registerMetaComponent<PointLightComponent>();
  registerMetaComponent<IdentifierComponent>();
  registerMetaComponent<TransformComponent>();
  registerMetaComponent<OutlineComponent>();
  registerMetaComponent<IndexComponent>();
  registerMetaComponent<MeshComponent>();

  registerMetaEvent<LuaEvent>();
  registerMetaEvent<LuaEventHandler<LuaEvent>>();

  DynamicRigidbodyComponent::createLuaBindings( lua );
  DirectionalLightComponent::createLuaBindings( lua );
  StaticRigidbodyComponent::createLuaBindings( lua );
  PointLightComponent::createLuaBindings( lua );
  IdentifierComponent::createLuaBindings( lua );
  TransformComponent::createLuaBindings( lua );
  OutlineComponent::createLuaBindings( lua );
  IndexComponent::createLuaBindings( lua );
  MeshComponent::createLuaBindings( lua );
}

} // namespace kogayonon_core
