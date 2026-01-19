#include "core/event/scene_events.hpp"

namespace kogayonon_core
{
SelectEntityEvent::SelectEntityEvent()
    : m_entity{ entt::null }
    , m_source{ SelectEntityEventSource::None }
{
}

SelectEntityEvent::SelectEntityEvent( entt::entity ent, SelectEntityEventSource source )
    : m_entity{ ent }
    , m_source{ source }
{
}

auto SelectEntityEvent::getEntityId() const -> entt::entity
{
  return m_entity;
}

auto SelectEntityEvent::getEventSource() const -> SelectEntityEventSource
{
  return m_source;
}

} // namespace kogayonon_core