#pragma once
#include <entt/entt.hpp>
#include "core/event/event.hpp"

namespace kogayonon_core
{
class SelectEntityEvent : public IEvent
{
public:
  SelectEntityEvent();
  explicit SelectEntityEvent( entt::entity ent );

  entt::entity getEntity() const;

private:
  entt::entity m_entity;
};

class SelectEntityInViewportEvent : public IEvent
{
public:
  SelectEntityInViewportEvent();
  explicit SelectEntityInViewportEvent( entt::entity ent );

  entt::entity getEntity() const;

private:
  entt::entity m_entity;
};

class SaveSceneEvent : public IEvent
{
public:
  SaveSceneEvent() = default;

private:
};

} // namespace kogayonon_core