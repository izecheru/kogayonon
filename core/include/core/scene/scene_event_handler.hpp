#pragma once
#include <entt/entt.hpp>

namespace core
{
class DeleteEntityEvent;
class AddEntityEvent;
class SelectEntityEvent;
class EventDispatcher;
} // namespace core

namespace core
{
class SceneEventHandler
{
public:
  SceneEventHandler( core::EventDispatcher* pDispatcher );
  ~SceneEventHandler() = default;

  auto getCurrentEntityId() const -> entt::entity;

  auto onDeleteEntity( const core::DeleteEntityEvent& e ) -> void;
  auto onAddEntity( const core::AddEntityEvent& e ) -> void;
  auto onSelectEntity( const core::SelectEntityEvent& e ) -> void;

private:
  entt::entity m_selectedEntity{ entt::null };
};
} // namespace core