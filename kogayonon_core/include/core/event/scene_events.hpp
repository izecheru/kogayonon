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
public:
  SelectEntityEvent( entt::entity ent );

  entt::entity getEntity() const;

  EVENT_CLASS_TYPE( SelectedEntity )
private:
  entt::entity m_entity;
};
} // namespace kogayonon_core