#include "core/systems/scripting_system.hpp"
#include <filesystem>
#include <fstream>
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
#include "window/window.hpp"

using namespace kogayonon_utilities;
using namespace kogayonon_window;
namespace fs = std::filesystem;

static void test()
{
  std::cout << "c++ called\n";
}

namespace kogayonon_core
{
ScriptingSystem::ScriptingSystem()
    : m_luaState{ sol::state{} }
    , m_init{ false }
{
  // make the main script object and add it to the registry
  auto mainScript = std::make_shared<MainScriptFuncs>();

  // move to main registry
  MainRegistry::getInstance().addToContext<std::shared_ptr<MainScriptFuncs>>( std::move( mainScript ) );

  auto currentPath = std::filesystem::absolute( "." ) / "resources\\scripts\\main.lua";
  spdlog::info( currentPath.string() );
  assert( fs::exists( currentPath ) == true && "main.lua MUST exist in the resources/scripts folder" );
  m_luaState.open_libraries( sol::lib::base, sol::lib::package, sol::lib::string );

  // register all usretypes and expose them to lua
  registerBindings( m_luaState );
  loadMainScript( currentPath.string() );
}

void ScriptingSystem::registerBindings( sol::state& lua )
{
  Entity::createLuaBindings( lua );
  Window::createLuaBindings( lua );
  Registry::createLuaBindings( lua );
  TimeTracker::createLuaBindings( lua );
  EventDispatcher::createLuaBindings( lua );
  DynamicRigidbodyComponent::createLuaBindings( lua );
  DirectionalLightComponent::createLuaBindings( lua );
  StaticRigidbodyComponent::createLuaBindings( lua );
  PointLightComponent::createLuaBindings( lua );
  IdentifierComponent::createLuaBindings( lua );
  TransformComponent::createLuaBindings( lua );
  OutlineComponent::createLuaBindings( lua );
  IndexComponent::createLuaBindings( lua );
  MeshComponent::createLuaBindings( lua );

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
}

bool ScriptingSystem::isInit() const
{
  return m_init;
}

void ScriptingSystem::loadMainScript( const std::string& path )
{
  // load the script
  m_luaState.safe_script_file( path );
  // check for update and render funcs
  sol::table mainLua = m_luaState["main"];

  sol::function init = mainLua["init"];
  sol::function update = mainLua["update"];
  sol::function render = mainLua["render"];

  auto& mainScript = MainRegistry::getInstance().getMainScriptFuncs();

  mainScript->init = init;
  mainScript->update = update;
  mainScript->render = render;

  mainLua.set_function( "render", &test );
  mainScript->render = mainLua["render"];
  mainScript->render();
  m_init = true;
}

auto ScriptingSystem::getLuaState() -> sol::state&
{
  return m_luaState;
}

} // namespace kogayonon_core
