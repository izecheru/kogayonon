#pragma once
#include <entt/entt.hpp>
#include "core/event/event.hpp"

namespace kogayonon_core
{
/**
 * @brief Type of event for entity change in SceneHierarchyWindow to propagate to the PropertiesWindow
 */
class SelectEntityEvent : public IEvent
{
  // todo specialise this event with some kind of From variable, From = "viewport" or From = "scene_hierarchy" to know
  // which handler should process it since if i select an entity in the viewport, scene hierarchy should also get it and
  // the other way round

public:
  SelectEntityEvent();
  explicit SelectEntityEvent( entt::entity ent );

  entt::entity getEntity() const;

  EVENT_CLASS_TYPE( SelectedEntity )

private:
  entt::entity m_entity;
};
} // namespace kogayonon_core