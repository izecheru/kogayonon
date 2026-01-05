#pragma once
#include <string>
#include "core/ecs/entity_types.hpp"

namespace kogayonon_core
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

struct IdentifierComponent
{
  std::string name{};
  EntityType type{};
  std::string group{};
};
} // namespace kogayonon_core