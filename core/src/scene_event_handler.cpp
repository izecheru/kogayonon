#include "core/scene/scene_event_handler.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"

core::SceneEventHandler::SceneEventHandler( EventDispatcher* pDispatcher )
{
  pDispatcher->addHandler<core::DeleteEntityEvent, &SceneEventHandler::onDeleteEntity>( *this );
  pDispatcher->addHandler<core::SelectEntityEvent, &SceneEventHandler::onSelectEntity>( *this );
  pDispatcher->addHandler<core::AddEntityEvent, &SceneEventHandler::onAddEntity>( *this );
}

auto core::SceneEventHandler::getCurrentEntityId() const -> entt::entity
{
  return m_selectedEntity;
}

auto core::SceneEventHandler::onDeleteEntity( const core::DeleteEntityEvent& e ) -> void
{
  if ( m_selectedEntity == entt::null )
  {
    return;
  }

  auto sceneManager = core::MainRegistry::getInstance().getSceneManager();
  auto scene = sceneManager->getCurrentScene();

  scene->removeEntity( m_selectedEntity );

  m_selectedEntity = entt::null;
}

auto core::SceneEventHandler::onAddEntity( const core::AddEntityEvent& e ) -> void
{
  auto sceneManager = core::MainRegistry::getInstance().getSceneManager();
  auto scene = sceneManager->getCurrentScene();
  m_selectedEntity = scene->addEntity();
}

auto core::SceneEventHandler::onSelectEntity( const core::SelectEntityEvent& e ) -> void
{
  m_selectedEntity = e.getEntityId();
}
