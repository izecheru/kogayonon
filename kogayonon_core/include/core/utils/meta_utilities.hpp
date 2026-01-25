#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <spdlog/spdlog.h>
#include "core/ecs/registry.hpp"

// many thanks to this man on github https://github.com/skaarj1989/entt-meets-sol2
// he defined those functions and i just had to understand them and from there try to implement my own

namespace kogayonon_core
{
/**
 * @brief Get the entt::id_type of the sol::object
 * @param obj The object we get from lua
 * @return entt::id_type of the object [ MUST BE REGISTERED IN meta_factory ]
 */
[[nodiscard]] static auto getTypeId( const sol::table& obj ) -> entt::id_type
{
  // the typeId is stored in the new_usertype for each object we expose to lua
  const auto f = obj["typeId"].get<sol::function>();
  assert( f.valid() && "type_id not exposed to lua!" );
  return f.valid() ? f().get<entt::id_type>() : -1;
}

/**
 * @brief Deduces the type a function received in lua
 * @tparam T Type of object
 * @param obj sol::type::table for Component or sol::type::number for Component.typeId
 * @return entt::id_type of that component so we can use in meta and call apropriate functions
 */
template <typename T>
[[nodiscard]] static auto deduceType( T&& obj ) -> entt::id_type
{
  switch ( obj.get_type() )
  {
  // if we have registry:has(entity, Transform.type_id()) in lua file then it is a number
  case sol::type::number:
    return obj.template as<entt::id_type>();
  // if we have registry:has(entity, Transform) then we have a table, like struct {}
  case sol::type::table:
    return getTypeId( obj );
  }
  assert( false );
  return -1;
}

/**
 * @brief Get the data field from meta object, getField<glm::vec3, float>(any,"x")
 * @tparam T Type of structure
 * @tparam TData Type of data that gets returned
 * @param object The meta_any object we retrieve data from
 * @param fieldName The name of the data member variable
 * @return The actual value of the data member
 */
template <typename T, typename TData>
static TData getField( entt::meta_any& object, const char* fieldName )
{
  auto type = entt::resolve<T>();
  auto field = type.data( entt::hashed_string::value( fieldName ) );

  if ( !field )
    throw std::runtime_error( "Field not found, register it with meta_factory" );

  return field.get( object ).cast<TData>();
}

/**
 * @brief Deduces types to every element from the variadic_args param
 * @param va
 * @return A std::set<entt::id_type> containing the deduced types of each sol::object
 */
static auto collectTypes( const sol::variadic_args& va ) -> std::set<entt::id_type>
{
  std::set<entt::id_type> types;
  std::transform( va.cbegin(), va.cend(), std::inserter( types, types.begin() ), []( const auto& obj ) {
    return deduceType( obj );
  } );
  return types;
}

// https://github.com/skypjack/entt/wiki/Crash-Course:-runtime-reflection-system
/**
 * @brief Invokes the function with the entt::id_type for a specific entt::meta_type [ REGISTER THE TYPES WITH
 * entt::meta_factory<Type>()!!! ]
 * @tparam ...Args
 * @param metaType The type we need to look for a function for
 * @param funcId Id of the function, let's say we registered "func"_hs, this is the param that you place in here
 * @param ...args Any needed argument by the meta_function registered in the factory
 * @return
 */
template <typename... Args>
static inline auto invokeMetaFunc( entt::meta_type metaType, entt::id_type funcId, Args&&... args ) -> entt::meta_any
{
  if ( !metaType )
  {
    std::cout << "meta type is not registered in entt::meta";
  }
  else
  {
    if ( auto&& metaFunction = metaType.func( funcId ); metaFunction )
      return metaFunction.invoke( {}, std::forward<Args>( args )... );
  }
  return entt::meta_any{};
}

template <typename... Args>
static inline auto invokeMetaFunc( entt::id_type typeId, entt::id_type funcId, Args&&... args ) -> entt::meta_any
{
  return invokeMetaFunc( entt::resolve( typeId ), funcId, std::forward<Args>( args )... );
}

} // namespace kogayonon_core