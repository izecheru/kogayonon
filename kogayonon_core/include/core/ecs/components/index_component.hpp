#pragma once
#include <cstdint>
#include <entt/entt.hpp>
#include <sol/sol.hpp>

namespace kogayonon_core
{
struct IndexComponent
{
  uint32_t index;

  static void createLuaBindings( sol::state& lua )
  {
    lua.new_usertype<IndexComponent>(
      "IndexComponent",
      "typeId",
      entt::type_hash<IndexComponent>::value,
      sol::call_constructor,
      sol::factories( []( const uint32_t index ) { return IndexComponent{ .index = index }; } ),
      "index",
      &IndexComponent::index );
  }
};
} // namespace kogayonon_core