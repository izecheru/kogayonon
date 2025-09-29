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
#include "imgui_utils/imgui_utils.h"
#include "utilities/asset_manager/asset_manager.hpp"

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
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };
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

void EntityPropertiesWindow::drawEnttProperties( std::shared_ptr<kogayonon_core::Scene> scene )
{
  kogayonon_core::Entity entity( scene->getRegistry(), m_entity );
  auto pModel = entity.tryGetComponent<kogayonon_core::ModelComponent>();
  auto pTexture = entity.tryGetComponent<kogayonon_core::TextureComponent>();

  // entity always has a name
  auto pNameComp = entity.tryGetComponent<kogayonon_core::NameComponent>();
  if ( pNameComp )
  {
    ImGui::Text( "%s", pNameComp->name.c_str() );
  }

  if ( ImGui::BeginCombo( "##test", "Add component" ) )
  {
    // if we dont have a model we can add
    if ( !pModel )
    {
      if ( ImGui::MenuItem( "Model" ) )
      {
        entity.addComponent<kogayonon_core::ModelComponent>( ASSET_MANAGER()->getModel( "default" ) );
      }
    }

    // if we dont have a texture we can add
    if ( !pTexture )
    {
      if ( ImGui::MenuItem( "Texture" ) )
      {
        entity.addComponent<kogayonon_core::TextureComponent>( ASSET_MANAGER()->getTexture( "default" ) );
      }
    }

    ImGui::EndCombo();
  }

  // has textures?
  ImGui_Utils::addPaddedGui( [this, &entity]() { drawTextureComponent( entity ); }, ImVec2{ 1.0f, 10.0f } );

  // has transform component?
  ImGui_Utils::addPaddedGui( [this, &entity]() { drawTransformComponent( entity ); }, ImVec2{ 1.0f, 10.0f } );

  // has model component?
  ImGui_Utils::addPaddedGui( [this, &entity]() { drawModelComponent( entity ); }, ImVec2{ 1.0f, 10.0f } );
}

void EntityPropertiesWindow::onEntitySelect( const kogayonon_core::SelectEntityEvent& e )
{
  m_entity = e.getEntity();
}

void EntityPropertiesWindow::drawTextureComponent( kogayonon_core::Entity& ent )
{
  auto pTextureComponent = ent.tryGetComponent<kogayonon_core::TextureComponent>();

  if ( !pTextureComponent )
    return;

  ImGui::Text( "Texture" );
  ImGui::Image( (ImTextureID)pTextureComponent->getTextureId(), ImVec2{ 100.0f, 100.0f } );

  // can remove it
  if ( ImGui::BeginPopupContextItem( "TextureComponentContextMenu",
                                     ImGuiPopupFlags_NoOpenOverItems | ImGuiPopupFlags_MouseButtonRight ) )
  {
    if ( ImGui::MenuItem( "Remove" ) )
    {
      ent.removeComponent<kogayonon_core::TextureComponent>();
    }
    ImGui::EndPopup();
  }
}

void EntityPropertiesWindow::drawModelComponent( kogayonon_core::Entity& ent )
{
  auto pModelComponent = ent.tryGetComponent<kogayonon_core::ModelComponent>();
  if ( !pModelComponent )
    return;

  auto model = pModelComponent->pModel.lock();

  if ( !model )
    return;

  ImGui::Text( "Model component" );

  if ( ImGui::Button( "Remove" ) )
  {
    // they are tied together
    ent.removeComponent<kogayonon_core::ModelComponent>();
    ent.removeComponent<kogayonon_core::TransformComponent>();
  }
  ImGui::Text( "Model has %d meshes", model->getMeshes().size() );
}

void EntityPropertiesWindow::drawTransformComponent( kogayonon_core::Entity& ent )
{
  auto transformComponent = ent.tryGetComponent<kogayonon_core::TransformComponent>();
  if ( !transformComponent )
    return;

  ImGui::Text( "Transformation" );
  auto& pos = transformComponent->pos;
  ImGui::SliderFloat( "x", &pos.x, 0.0f, 100.0f );
  ImGui::SliderFloat( "y", &pos.y, 0.0f, 100.0f );
  ImGui::SliderFloat( "z", &pos.z, 0.0f, 100.0f );
  ImGui::SliderFloat( "scale", &transformComponent->scale.x, 0.0f, 100.0f );
  transformComponent->updateMatrix();
}
} // namespace kogayonon_gui