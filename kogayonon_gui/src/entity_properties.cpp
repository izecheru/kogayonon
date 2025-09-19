#include "gui/entity_properties.hpp"
#include <entt/entt.hpp>
#include "core/ecs/components/name_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_manager.hpp"
#include "core/event/scene_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"

namespace kogayonon_gui
{
EntityPropertiesWindow::EntityPropertiesWindow( std::string name )
    : ImGuiWindow( std::move( name ) )
{
  EVENT_MANAGER()->listenToEvent<kogayonon_core::ChangeEntityEvent>( [this]( const kogayonon_core::IEvent& e ) -> bool {
    return this->onEntityChange( (kogayonon_core::ChangeEntityEvent&)e );
  } );
}

void EntityPropertiesWindow::draw()
{
  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    ImGui::End();
    return;
  }
  m_pCurrentScene = kogayonon_core::SceneManager::getCurrentScene();
  if ( auto scene = m_pCurrentScene.lock() )
  {
    // auto ent = kogayonon_core::SceneContext::getSelectedEntity();
    if ( m_entity != entt::null )
    {
      kogayonon_core::Entity entity( scene->getRegistry(), m_entity );
      if ( auto* pNameComp = entity.tryGetComponent<kogayonon_core::NameComponent>() )
      {
        ImGui::Text( "Entity name: %s", pNameComp->name.c_str() );
      }
    }
    else
    {
      ImGui::Text( "Entity name: {select an entity}" );
    }
  }
  ImGui::End();
}

bool EntityPropertiesWindow::onEntityChange( kogayonon_core::ChangeEntityEvent& e )
{
  m_entity = e.getEntity();
  return true;
}
} // namespace kogayonon_gui