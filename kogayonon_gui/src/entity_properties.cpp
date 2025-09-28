#include "gui/entity_properties.hpp"
#include <entt/entt.hpp>
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/name_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"

namespace kogayonon_gui
{
EntityPropertiesWindow::EntityPropertiesWindow( std::string name )
    : ImGuiWindow{ std::move( name ) }
    , m_entity{ entt::null }
{
  EVENT_DISPATCHER()->addHandler<kogayonon_core::SelectEntityEvent, &EntityPropertiesWindow::onEntitySelect>( *this );
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
    if ( m_entity != entt::null )
    {
      drawEnttProperties( scene );
    }
    else
    {
      ImGui::Text( "No entity is currently selected" );
    }
  }
  ImGui::End();
}

void EntityPropertiesWindow::drawEnttProperties( std::shared_ptr<kogayonon_core::Scene> scene ) const
{
  kogayonon_core::Entity entity( scene->getRegistry(), m_entity );
  if ( auto pNameComp = entity.tryGetComponent<kogayonon_core::NameComponent>() )
  {
    ImGui::Text( "Name: %s", pNameComp->name.c_str() );
  }
  if ( auto pTextureComp = entity.tryGetComponent<kogayonon_core::TextureComponent>() )
  {
    ImGui::Image( (ImTextureID)pTextureComp->getTextureId(), ImVec2{ 220.0f, 220.0f } );
  }
  auto pModelComp = entity.tryGetComponent<kogayonon_core::ModelComponent>();

  if ( !pModelComp )
    return;

  const auto& model = pModelComp->pModel.lock();

  if ( !model )
    return;

  auto transform = entity.tryGetComponent<kogayonon_core::TransformComponent>();

  if ( transform )
  {
    auto& pos = transform->pos;
    ImGui::SliderFloat( "x", &pos.x, 0.0f, 100.0f );
    ImGui::SliderFloat( "y", &pos.y, 0.0f, 100.0f );
    ImGui::SliderFloat( "z", &pos.z, 0.0f, 100.0f );
    ImGui::SliderFloat( "scale", &transform->scale.x, 0.0f, 100.0f );
    transform->updateMatrix();
  }

  const auto& meshes = model->getMeshes();
  int amount = model->getAmount();
  ImGui::Text( "Mesh vector size <%d>", static_cast<int>( meshes.size() ) );
  ImGui::Text( "Current instances number %d", amount );
  if ( ImGui::Button( "Add 100 instances of this object" ) )
  {
    model->setAmount( amount + 100 );
  }
}

void EntityPropertiesWindow::onEntitySelect( const kogayonon_core::SelectEntityEvent& e )
{
  m_entity = e.getEntity();
}
} // namespace kogayonon_gui