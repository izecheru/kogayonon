#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <string>
#include <yaml-cpp/yaml.h>
#include "core/ecs/entity_types.hpp"
#include "utilities/utils/yaml_utils.hpp"

namespace core
{

/**
 * @brief Transforms type of entity to string
 * @param type Type of the entity
 * @return Returns a string equivalent to the entity type
 */
static std::string typeToString( EntityType type )
{
  switch ( type )
  {
  case EntityType::None:
    return "None";
  case EntityType::Empty:
    return "Empty";
  case EntityType::Camera:
    return "Camera";
  case EntityType::EditorCamera:
    return "EditorCamera";
  case EntityType::Light:
    return "Light";
  case EntityType::Object:
    return "Object";
  case EntityType::UIelement:
    return "UIelement";
  }
  return "Invalid type";
}

static EntityType stringToType( const std::string& str )
{
  if ( str == "None" )
    return EntityType::None;
  if ( str == "Empty" )
    return EntityType::Empty;
  if ( str == "Camera" )
    return EntityType::Camera;
  if ( str == "EditorCamera" )
    return EntityType::EditorCamera;
  if ( str == "Light" )
    return EntityType::Light;
  if ( str == "Object" )
    return EntityType::Object;
  if ( str == "UIelement" )
    return EntityType::UIelement;

  return EntityType::None; // Default or invalid value
}

struct IdentifierComponent
{
  std::string name{};
  EntityType type{};
  std::string group{};

  static void createLuaBindings( sol::state& lua )
  {
    lua.new_enum<EntityType>( "EntityType",
                              { { "Camera", EntityType::Camera },
                                { "EditorCamera", EntityType::EditorCamera },
                                { "Light", EntityType::Light },
                                { "Object", EntityType::Object },
                                { "None", EntityType::None } } );

    lua.new_usertype<IdentifierComponent>( "IdentifierComponent",
                                           "typeId",
                                           entt::type_hash<IdentifierComponent>::value,
                                           sol::call_constructor,
                                           sol::factories( []( const std::string& name ) {
                                             return IdentifierComponent{
                                               .name = name, .type = EntityType::Object, .group = "MainGroup" };
                                           } ),
                                           "name",
                                           &IdentifierComponent::name,
                                           "type",
                                           &IdentifierComponent::type,
                                           "group",
                                           &IdentifierComponent::group );
  }
};

} // namespace core
