#pragma once
#include <entt/entt.hpp>
#include "core/event/event.hpp"

namespace kogayonon_core
{
/**
 * @brief Type of event for entity change in SceneHierarchyWindow to propagate to the PropertiesWindow
 */
class ChangeEntityEvent : public IEvent
{
public:
  ChangeEntityEvent( entt::entity ent );

  entt::entity getEntity() const;

  EVENT_CLASS_TYPE( EntityChanged )
private:
  entt::entity m_entity;
};
} // namespace kogayonon_core