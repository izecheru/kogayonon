#include "core/ecs/registry.hpp"
#include "core/ecs/entity.hpp"
#include "core/utils/meta_utilities.hpp"

namespace kogayonon_core
{
void Registry::createLuaBindings( sol::state& lua )
{

  lua.new_usertype<entt::runtime_view>( "runtime_view",
                                        sol::no_constructor,
                                        "size_hint",
                                        &entt::runtime_view::size_hint,
                                        "contains",
                                        &entt::runtime_view::contains,
                                        "each",
                                        []( const entt::runtime_view& self, const sol::function& callback ) {
                                          if ( callback.valid() )
                                          {
                                            for ( auto entity : self )
                                              callback( entity );
                                          }
                                        } );
  lua.new_usertype<Registry>(
    "Registry",
    sol::call_constructor,
    sol::constructors<Registry()>(),

    "createEntity",
    sol::overload( []( Registry& self ) { return self.createEntity(); },
                   []( Registry& self, const std::string& name ) { return Entity{ &self, name }; } ),

    "removeEntity",
    sol::overload(
      // entt::entity overload
      []( Registry& self, entt::entity entity ) { return self.removeEntity( entity ); },

      // Entity overload
      []( Registry& self, Entity& entity ) { return self.removeEntity( entity.getEntityId() ); } ),

    "runtimeView",
    []( Registry& self, const sol::variadic_args& va ) {
      const auto types = collectTypes( va );
      auto view = entt::runtime_view{};
      for ( auto&& [componentId, storage] : self.storage() )
      {
        // if view was made with <MeshComponent> and types has the entt::type_hash of that, the view will
        // iterate through the storage
        if ( types.find( componentId ) != types.cend() )
          view.iterate( storage );
      }
      return view;
    } );
}
} // namespace kogayonon_core
