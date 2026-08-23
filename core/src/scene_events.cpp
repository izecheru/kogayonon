#include "core/event/scene_events.hpp"

namespace core
{
SelectEntityEvent::SelectEntityEvent()
    : m_entity{ entt::null }
    , m_source{ SelectEntityEventSource::None }
{
}

SelectEntityEvent::SelectEntityEvent( const entt::entity& ent, const SelectEntityEventSource& source )
    : m_entity{ ent }
    , m_source{ source }
{
}

SelectEntityEvent::SelectEntityEvent( const SelectEntityEventSource& source )
    : m_entity{ entt::null }
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

DeleteEntityEvent::DeleteEntityEvent( const entt::entity& entityId )
    : m_entityId{ entityId }
{
}

} // namespace core