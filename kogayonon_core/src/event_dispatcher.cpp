#include "core/event/event_dispatcher.hpp"
#include "core/utils/meta_utilities.hpp"

namespace kogayonon_core
{
void EventDispatcher::createLuaBindings( sol::state& lua )
{
  lua.new_usertype<LuaEventHandler<LuaEvent>>(
    "LuaEventHandler",
    "typeId",
    entt::type_hash<LuaEventHandler<LuaEvent>>::value,
    "eventType",
    entt::type_hash<LuaEvent>::value,
    sol::call_constructor,
    sol::factories( []( const sol::function& func ) { return LuaEventHandler<LuaEvent>{ .callback = func }; } ),
    "release",
    &LuaEventHandler<LuaEvent>::release );

  lua.new_usertype<LuaEvent>( "LuaEvent",
                              "typeId",
                              &entt::type_hash<LuaEvent>::value,
                              sol::call_constructor,
                              sol::factories( []( const sol::object& data ) { return LuaEvent{ .data = data }; } ),
                              "data",
                              &LuaEvent::data );

  lua.new_usertype<EventDispatcher>(
    "EventDispatcher",
    sol::call_constructor,
    // sol::constructors<EventDispatcher()>(),
    sol::factories( [&]( sol::this_state currentState ) -> sol::object {
      // TODO find a nice way to pass a dispatcher that lives on c++ side already
      // if lua == true then the dispatcher is local to lua
      // if ( lua )
      return sol::make_object( currentState, EventDispatcher{} );

      // this is a reference
      // return dispatcher;
    } ),
    "hasHandlers",
    []( EventDispatcher& self, const sol::table& eventTypeOrId ) {
      auto any = invokeMetaFunc( deduceType( eventTypeOrId ), "has_handlers"_hs, self );
      return any ? any.cast<bool>() : false;
    },
    "removeHandler",
    []( EventDispatcher& self, const sol::object& eventTypeOrId, const sol::object& listener ) {
      invokeMetaFunc( getTypeId( eventTypeOrId ), "remove_handler"_hs, self, listener );
    },
    "addHandler",
    []( EventDispatcher& self, const sol::object& eventTypeOrId, const sol::object& listener, sol::this_state s ) {
      if ( !listener.valid() )
      {
        return entt::meta_any{};
      }
      return invokeMetaFunc( getTypeId( eventTypeOrId ), "add_handler"_hs, self, listener );
    },
    "dispatchEvent",
    []( EventDispatcher& self, const sol::table& event ) {
      invokeMetaFunc( getTypeId( event ), "dispatch_event"_hs, self, event );
    } );
}
} // namespace kogayonon_core
