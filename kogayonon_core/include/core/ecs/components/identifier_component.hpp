#pragma once
#include <string>

namespace kogayonon_core
{

// now we can add functionality based on entity type, we can filter them
enum class EntityType
{
  None = 0,
  Camera,
  EditorCamera,
  Object,
  Light,
  UIelement,
  Empty
};

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