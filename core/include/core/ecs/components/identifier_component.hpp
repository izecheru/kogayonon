#pragma once
#include <entt/entt.hpp>
#include <sol/sol.hpp>
#include <yaml-cpp/yaml.h>
#include "core/ecs/entity_types.hpp"
#include "precompiled/pch.hpp"
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
};

} // namespace core
