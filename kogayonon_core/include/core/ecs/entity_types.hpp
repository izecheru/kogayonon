#pragma once

// now we can add functionality based on entity type, we can filter them
namespace kogayonon_core
{
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
} // namespace kogayonon_core
