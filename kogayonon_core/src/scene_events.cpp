#include "core/event/scene_events.hpp"

namespace kogayonon_core
{
ChangeEntityEvent::ChangeEntityEvent( entt::entity ent )
{
  m_entity = ent;
}

entt::entity ChangeEntityEvent::getEntity() const
{
  return m_entity;
}
} // namespace kogayonon_core
