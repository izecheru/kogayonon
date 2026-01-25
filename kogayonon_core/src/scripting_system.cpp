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
#include "core/ecs/registry.hpp"
#include "core/event/event_dispatcher.hpp"

namespace kogayonon_core
{
ScriptingSystem::ScriptingSystem()
    : m_luaState{ sol::state{} }
{
  auto absPath = std::filesystem::absolute( "." );
  m_luaState.open_libraries( sol::lib::base, sol::lib::package, sol::lib::string );

  absPath = absPath / "resources\\scripts\\main.lua";
  if ( !loadMainScriptFile( absPath.string(), m_luaState ) )
  {
    spdlog::error( "could not load the main script file" );
  }
}

ScriptingSystem::ScriptingSystem( sol::state& lua, const std::string& scriptPath )
{
  if ( !loadMainScriptFile( scriptPath, lua ) )
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
  lua.safe_script_file( path );
}

void ScriptingSystem::registerBindings( sol::state& lua )
{
  Entity::createLuaBindings( lua );
  Registry::createLuaBindings( lua );
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