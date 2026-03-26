#pragma once

// now we can add functionality based on entity type, we can filter them
namespace core
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
} // namespace core
