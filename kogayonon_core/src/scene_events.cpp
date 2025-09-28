#include "core/event/scene_events.hpp"

namespace kogayonon_core
{
SelectEntityEvent::SelectEntityEvent()
    : m_entity{ entt::null }
{
}

SelectEntityEvent::SelectEntityEvent( entt::entity ent )
    : m_entity{ ent }
{
}

entt::entity SelectEntityEvent::getEntity() const
{
  return m_entity;
}
} // namespace kogayonon_core