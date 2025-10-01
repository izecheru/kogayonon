#include "gui/entity_properties.hpp"
#include <entt/entt.hpp>
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include "core/ecs/components/index_component.h"
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
#include "utilities/math/math.hpp"
using namespace kogayonon_utilities;

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

  // change the entity name
  ImGui::TextUnformatted( "Change name" );
  ImGui::SameLine();
  if ( char buffer[50] = { 0 };
       ImGui::InputText( "##change_name", buffer, IM_ARRAYSIZE( buffer ), ImGuiInputTextFlags_EnterReturnsTrue ) )
  {
    std::string result{ buffer };
    pNameComp->name = result;
  }

  if ( ImGui::BeginCombo( "##test", "Add component" ) )
  {
    // if we dont have a texture we can add
    if ( !pTexture )
    {
      auto pTexture = ASSET_MANAGER()->getTexture( "default" );
      if ( ImGui::MenuItem( "Texture" ) )
      {
        if ( const auto& modelComponent = entity.tryGetComponent<kogayonon_core::ModelComponent>() )
        {
          const auto& model = modelComponent->pModel.lock();
          if ( model )
          {
            auto& meshes = model->getMeshes();
            for ( auto& mesh : meshes )
            {
              auto& textures = mesh.getTextures();

              // if the meshes do not have any texture we push back one
              if ( textures.size() == 0 )
              {
                textures.push_back( pTexture.lock()->getTextureId() );
              }
              else
              {
                // if we have textures we assign all of the ids to the one we just loaded
                for ( int i = 0; i < textures.size(); ++i )
                {
                  if ( auto texture = pTexture.lock() )
                    textures.at( i ) = texture->getTextureId();
                }
              }
            }
          }
        }
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

void EntityPropertiesWindow::drawTextureComponent( kogayonon_core::Entity& ent ) const
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

void EntityPropertiesWindow::drawModelComponent( kogayonon_core::Entity& ent ) const
{
  auto pModelComponent = ent.tryGetComponent<kogayonon_core::ModelComponent>();
  if ( !pModelComponent )
    return;

  auto model = pModelComponent->pModel.lock();

  if ( !model )
    return;

  ImGui::Text( "Model component" );
  ImGui::Text( "Model has %d meshes", model->getMeshes().size() );
}

void EntityPropertiesWindow::drawTransformComponent( kogayonon_core::Entity& ent ) const
{
  const auto& transformComponent = ent.tryGetComponent<kogayonon_core::TransformComponent>();
  if ( !transformComponent )
    return;

  // if it has transform, it definetely has a model component
  const auto& modelComponent = ent.tryGetComponent<kogayonon_core::ModelComponent>();

  ImGui::Text( "Transformation" );
  bool changed = false;
  auto& pos = transformComponent->pos;
  auto& scale = transformComponent->scale;
  changed |= ImGui::SliderFloat( "x", &pos.x, 0.0f, 100.0f );
  changed |= ImGui::SliderFloat( "y", &pos.y, 0.0f, 100.0f );
  changed |= ImGui::SliderFloat( "z", &pos.z, 0.0f, 100.0f );

  ImGui::Text( "Scale" );
  changed |= ImGui::SliderFloat( "##scale x", &scale.x, 1.0f, 100.0f );
  changed |= ImGui::SliderFloat( "##scale y", &scale.y, 1.0f, 100.0f );
  changed |= ImGui::SliderFloat( "##scale z", &scale.z, 1.0f, 100.0f );

  if ( changed )
  {
    auto scene = m_pCurrentScene.lock();

    if ( !scene )
      return;

    transformComponent->dirty = true;
    transformComponent->updateMatrix();

    // we need the index of the instance
    const auto& indexComponent = ent.getComponent<kogayonon_core::IndexComponent>();

    // get the instance data of this model
    const auto data = scene->getData( modelComponent->pModel.lock().get() );

    // get the instance matrix
    data->instanceMatrices.at( indexComponent.index ) = transformComponent->modelMatrix;

    // if we have only one instance it works

    // if we have a multiple instances
    if ( data->count > 1 )
    {
      scene->setupMultipleInstances( data );
    }
  }
}
} // namespace kogayonon_gui