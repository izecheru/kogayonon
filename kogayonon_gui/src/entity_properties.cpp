#include "gui/entity_properties.hpp"
#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <imgui_stdlib.h>
#include <physx/extensions/PxRigidActorExt.h>
#include <physx/extensions/PxRigidBodyExt.h>
#include <physx/extensions/PxSimpleFactory.h>
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/identifier_component.hpp"
#include "core/ecs/components/index_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/pointlight_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/event_emitter.hpp"
#include "core/event/scene_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "imgui_utils/imgui_utils.h"
#include "physics/nvidia_physx.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/math/math.hpp"
#include "utilities/task_manager/task_manager.hpp"

using namespace kogayonon_utilities;
using namespace kogayonon_core;

namespace kogayonon_gui
{
EntityPropertiesWindow::EntityPropertiesWindow( std::string name )
    : ImGuiWindow{ std::move( name ) }
    , m_selectedEntity{ entt::null }
{
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<SelectEntityEvent, &EntityPropertiesWindow::onEntitySelect>( *this );
}

void EntityPropertiesWindow::draw()
{
  ImGui_Utils::ScopedPadding padd{ ImVec2{ 10.0f, 10.0f } };

  if ( !begin() )
    return;

  auto pScene = SceneManager::getCurrentScene();
  if ( auto scene = pScene.lock() )
  {
    if ( m_selectedEntity != entt::null )
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

void EntityPropertiesWindow::drawDynamicRigidbodyComponent( kogayonon_core::Entity& ent ) const
{
  if ( !ent.hasComponent<DynamicRigidbodyComponent>() )
    return;

  ImGui::SeparatorText( "Dynamic rigidbody" );
  auto& dyanmicRigidbody = ent.getComponent<DynamicRigidbodyComponent>();
  auto name = dyanmicRigidbody.pBody->getName();
  ImGui::Text( "%s", name );
}

void EntityPropertiesWindow::drawStaticRigidbodyComponent( kogayonon_core::Entity& ent ) const
{
  if ( !ent.hasComponent<StaticRigidbodyComponent>() )
    return;

  ImGui::SeparatorText( "Static rigidbody" );
  auto& staticRigidbody = ent.getComponent<StaticRigidbodyComponent>();
  auto name = staticRigidbody.pBody->getName();
  ImGui::Text( "%s", name );
}

void EntityPropertiesWindow::drawLightMenu( Entity& ent )
{
  auto scene = SceneManager::getCurrentScene().lock();
  ImGui::SeparatorText( "Lighting" );
  if ( ImGui::BeginMenu( "Light" ) )
  {
    if ( !ent.hasComponent<PointLightComponent>() )
    {
      if ( ImGui::MenuItem( "Point light" ) )
      {
        scene->addPointLight( ent.getEntityId() );
      }
    }
    ImGui::EndMenu();
  }
}

void EntityPropertiesWindow::drawEnttProperties( std::shared_ptr<Scene> scene )
{
  Entity entity{ scene->getRegistry(), m_selectedEntity };

  if ( auto pIdentifierComponent = entity.tryGetComponent<IdentifierComponent>() )
  {
    ImGui::SeparatorText( "Entity idenfitification" );
    ImGui::InputText( "##id", &pIdentifierComponent->name );
    ImGui::Text( "Group: %s", pIdentifierComponent->group.c_str() );
    ImGui::Text( "Type: %s", typeToString( pIdentifierComponent->type ).c_str() );
  }

  if ( ImGui::BeginPopupContextWindow( "##model_context" ) )
  {
    if ( ImGui::BeginMenu( "Add component" ) )
    {
      if ( !entity.hasComponent<MeshComponent>() )
      {
        if ( ImGui::MenuItem( "Mesh component" ) )
        {
          entity.addComponent<MeshComponent>();
        }
      }
      // has mesh component so we can add physics to it
      else
      {
        drawRigidbodyMenu( entity );
      }

      drawLightMenu( entity );

      if ( ImGui::MenuItem( "Texture component" ) )
      {
        // TODO add implementation for texture component
      }
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }

  drawMeshComponent( entity );
  drawTransformComponent( entity );
  drawPointLightComponent( entity );
  drawDirectionalLightComponent( entity );
  drawDynamicRigidbodyComponent( entity );
  drawStaticRigidbodyComponent( entity );
  drawMisc( entity );
}

void EntityPropertiesWindow::drawRigidbodyMenu( Entity& ent )
{
  ImGui::SeparatorText( "Physics" );
  if ( ImGui::BeginMenu( "Dynamic rigid body" ) )
  {
    if ( !ent.hasComponent<DynamicRigidbodyComponent>() )
    {
      if ( ImGui::MenuItem( "Box" ) )
      {
        ent.addComponent<DynamicRigidbodyComponent>();

        auto& box = ent.getComponent<DynamicRigidbodyComponent>();
        auto& transform = ent.getComponent<TransformComponent>();
        auto& nvidia = kogayonon_physics::NvidiaPhysx::getInstance();
        auto physics = nvidia.getPhysics();
        auto quat = glm::quat{ glm::radians( transform.rotation ) };

        // TODO change the box size somehow
        auto shape = physics->createShape(
          physx::PxBoxGeometry{ transform.scale.x, transform.scale.y, transform.scale.z }, *nvidia.getMaterial() );
        box.pBody = physics->createRigidDynamic(
          physx::PxTransform{ transform.translation.x, transform.translation.y, transform.translation.z,
                              physx::PxQuat{ quat.x, quat.y, quat.z, quat.w } } );
        box.pBody->attachShape( *shape );
        physx::PxRigidBodyExt::updateMassAndInertia( *box.pBody, 10.0f );
        nvidia.getScene()->addActor( *box.pBody );
        shape->release();
      }
      if ( ImGui::MenuItem( "Sphere" ) )
      {
        ent.addComponent<DynamicRigidbodyComponent>();

        auto& sphere = ent.getComponent<DynamicRigidbodyComponent>();
        auto& position = ent.getComponent<TransformComponent>().translation;
        auto& nvidia = kogayonon_physics::NvidiaPhysx::getInstance();
        auto physics = nvidia.getPhysics();

        auto shape = physics->createShape( physx::PxSphereGeometry{ 2.0f }, *nvidia.getMaterial() );
        sphere.pBody =
          physics->createRigidDynamic( physx::PxTransform{ physx::PxVec3{ position.x, position.y, position.z } } );
        sphere.pBody->attachShape( *shape );
        physx::PxRigidBodyExt::updateMassAndInertia( *sphere.pBody, 10.0f );
        nvidia.getScene()->addActor( *sphere.pBody );
        shape->release();
      }
    }
    ImGui::EndMenu();
  }

  if ( ImGui::BeginMenu( "Static rigid body" ) )
  {
    if ( !ent.hasComponent<StaticRigidbodyComponent>() )
    {
      if ( ImGui::MenuItem( "Box" ) )
      {
        ent.addComponent<StaticRigidbodyComponent>();

        auto& box = ent.getComponent<StaticRigidbodyComponent>();
        auto& transform = ent.getComponent<TransformComponent>();
        auto quat = glm::quat{ glm::radians( transform.rotation ) };
        auto& nvidia = kogayonon_physics::NvidiaPhysx::getInstance();
        auto physics = nvidia.getPhysics();

        // TODO change the box size somehow
        auto shape = physics->createShape(
          physx::PxBoxGeometry{ transform.scale.x, transform.scale.y, transform.scale.z }, *nvidia.getMaterial() );
        box.pBody = physics->createRigidStatic( physx::PxTransform{ transform.translation.x, transform.translation.y,
                                                                    transform.translation.z,
                                                                    physx::PxQuat{ quat.x, quat.y, quat.z, quat.w } } );
        box.pBody->attachShape( *shape );
        nvidia.getScene()->addActor( *box.pBody );
        shape->release();
      }
      if ( ImGui::MenuItem( "Sphere" ) )
      {
        ent.addComponent<StaticRigidbodyComponent>();

        auto& sphere = ent.getComponent<StaticRigidbodyComponent>();
        auto& position = ent.getComponent<TransformComponent>().translation;
        auto& nvidia = kogayonon_physics::NvidiaPhysx::getInstance();
        auto physics = nvidia.getPhysics();

        auto shape = physics->createShape( physx::PxSphereGeometry{ 2.0f }, *nvidia.getMaterial() );
        sphere.pBody =
          physics->createRigidStatic( physx::PxTransform{ physx::PxVec3{ position.x, position.y, position.z } } );
        sphere.pBody->attachShape( *shape );
        nvidia.getScene()->addActor( *sphere.pBody );
        shape->release();
      }
      if ( ImGui::MenuItem( "Plane" ) )
      {
        ent.addComponent<StaticRigidbodyComponent>();

        auto& plane = ent.getComponent<StaticRigidbodyComponent>();
        auto& position = ent.getComponent<TransformComponent>().translation;
        auto& nvidia = kogayonon_physics::NvidiaPhysx::getInstance();
        auto physics = nvidia.getPhysics();
        auto material = nvidia.getMaterial();

        plane.pBody = physx::PxCreatePlane( *physics, physx::PxPlane{ 0, 1, 0, 0 }, *material );

        nvidia.getScene()->addActor( *plane.pBody );
      }
    }
    ImGui::EndMenu();
  }
}

void EntityPropertiesWindow::onEntitySelect( const SelectEntityEvent& e )
{
  if ( m_selectedEntity == e.getEntityId() || e.getEventSource() == SelectEntityEventSource::PropertiesWindow )
    return;

  m_selectedEntity = e.getEntityId();
}

void EntityPropertiesWindow::drawTextureComponent( Entity& ent ) const
{
  const auto& pMeshComponent = ent.tryGetComponent<MeshComponent>();
  if ( !pMeshComponent )
    return;

  auto& mesh = pMeshComponent->pMesh;
  if ( !mesh )
    return;

  ImGui::BeginGroup();
  if ( ImGui::BeginDragDropTarget() )
  {
    manageTexturePayload( ImGui::AcceptDragDropPayload( "ASSET_DROP" ) );
    ImGui::EndDragDropTarget();
  }

  ImGui::BeginListBox( "##texture_list" );
  ImGui::EndListBox();
  ImGui::EndGroup();
}

void EntityPropertiesWindow::manageTexturePayload( const ImGuiPayload* payload ) const
{
}

void EntityPropertiesWindow::drawTextureContextMenu( std::vector<kogayonon_resources::Texture*>& textures,
                                                     int index ) const
{
  std::string label = std::format( "##{}", index );
  if ( ImGui::BeginPopupContextItem( label.c_str() ) )
  {
    if ( ImGui::MenuItem( "Delete texture" ) )
    {
      textures.erase( textures.begin() + index );
    }
    ImGui::EndPopup();
  }
}

void EntityPropertiesWindow::drawMeshComponent( Entity& ent )
{
  if ( !ent.hasComponent<MeshComponent>() )
    return;

  ImGui::SeparatorText( "Mesh" );
  auto pMeshComponent = ent.getComponent<MeshComponent>();
  if ( ImGui::BeginDragDropTarget() )
  {
    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ASSET_DROP" );
    manageModelPayload( payload );
    ImGui::EndDragDropTarget();
  }

  auto scene = SceneManager::getCurrentScene().lock();
  auto& mesh = pMeshComponent.pMesh;

  if ( !mesh )
    return;

  Entity entity{ scene->getRegistry(), m_selectedEntity };

  ImGui::Text( "Mesh has %d submeshes", mesh->getSubmeshes().size() );
  ImGui::TextWrapped( "Path: %s", mesh->getPath().c_str() );

  // TODO must draw some other stuff here idk, removing textures or something
  // draw every texture here, since each submesh references an already existing texture
  // we get duplicates so we insert them in an unordered set and then render them
  const auto& textures = mesh->getTextures();

  // this is static and changes when entity id changes
  static auto entityId = m_selectedEntity;
  static std::unordered_set<kogayonon_resources::Texture*> uniqueTextures;
  if ( entityId != m_selectedEntity )
  {
    entityId = m_selectedEntity;
    uniqueTextures.clear();

    for ( const auto& texture : textures )
    {
      uniqueTextures.insert( texture );
    }
  }

  for ( const auto& uniqueTex : uniqueTextures )
  {
    ImGui::Image( (ImTextureID)uniqueTex->getTextureId(), ImVec2{ 50.0f, 50.0f } );
    if ( ImGui::IsItemHovered() )
    {
      // should show here what submeshes uses this texture
      ImGui::BeginTooltip();
      ImGui::Image( (ImTextureID)uniqueTex->getTextureId(), ImVec2{ 250.0f, 250.0f } );
      ImGui::Text( "%s", uniqueTex->getName().c_str() );
      ImGui::Text( "%d/%d", uniqueTex->getWidth(), uniqueTex->getHeight() );
      ImGui::EndTooltip();
    }
  }

  if ( ImGui::Button( "Remove mesh" ) )
  {
    if ( !scene )
      return;

    scene->removeMeshFromEntity( ent.getEntityId() );
  }
}

void EntityPropertiesWindow::manageModelPayload( const ImGuiPayload* payload )
{
  if ( !payload )
  {
    return;
  }

  auto& assetManager = AssetManager::getInstance();
  auto data = static_cast<const char*>( payload->Data );
  std::string dropResult( data, payload->DataSize );
  std::filesystem::path p{ dropResult };

  auto scene = SceneManager::getCurrentScene();
  auto pScene = scene.lock();

  if ( !pScene )
    return;

  const auto& extension = p.extension().string();
  if ( extension.find( ".gltf" ) != std::string::npos )
  {
    const auto& pTaskManager = MainRegistry::getInstance().getTaskManager();

    auto entTemp = m_selectedEntity;
    Entity ent{ pScene->getRegistry(), m_selectedEntity };
    if ( ent.hasComponent<MeshComponent>() )
    {
      pScene->removeMeshFromEntity( ent.getEntityId() );
    }
    else
    {
      ent.addComponent<MeshComponent>();
    }

    const auto model = assetManager.getMesh( p.filename().string() );
    if ( model != nullptr )
    {
      pScene->addMeshToEntity( entTemp, model );
    }
    else
    {
      pTaskManager->enqueue( [entTemp, p, pScene]() {
        auto& assetManager = AssetManager::getInstance();
        Entity ent{ pScene->getRegistry(), entTemp };

        // both protected by their own mutexes
        const auto model = assetManager.addMesh( p.filename().string(), p.string() );
        pScene->addMeshToEntity( entTemp, model );
      } );
    }
  }
  else
  {
    spdlog::info( "format currently unsupported in viewport: {}", p.extension().string() );
  }
}

void EntityPropertiesWindow::drawMisc( kogayonon_core::Entity& ent )
{
  ImGui::SeparatorText( "Miscellaneous" );
  const auto& scene = SceneManager::getCurrentScene().lock();
}

void EntityPropertiesWindow::drawTransformComponent( Entity& ent ) const
{
  if ( !ent.hasComponent<TransformComponent>() )
    return;

  ImGui::SeparatorText( "Transform" );
  auto& transformComponent = ent.getComponent<TransformComponent>();

  // if it has transform, it definetely has a mesh component
  const auto& modelComponent = ent.tryGetComponent<MeshComponent>();

  bool changed = false;
  auto& translation = transformComponent.translation;
  auto& scale = transformComponent.scale;

  // since the first two are the largest we make the width = largest size
  // position
  // rotation
  // scale
  static auto textSize = ImGui::CalcTextSize( "Translation" );

  if ( ImGui::BeginTable( "##transform_table", 4 ) )
  {
    ImGui::TableSetupColumn( "label", ImGuiTableColumnFlags_WidthFixed, textSize.x );
    ImGui::TableSetupColumn( "x", ImGuiTableColumnFlags_WidthFixed, 100.0f );
    ImGui::TableSetupColumn( "y", ImGuiTableColumnFlags_WidthFixed, 100.0f );
    ImGui::TableSetupColumn( "z", ImGuiTableColumnFlags_WidthFixed, 100.0f );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Translation" );

    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Xtranslation", &translation.x, 0.1f, translation.x - 100.0f, translation.x + 100.0f,
                                 "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Ytranslation", &translation.y, 0.1f, translation.y - 100.0f, translation.y + 100.0f,
                                 "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Ztranslation", &translation.z, 0.1f, translation.z - 100.0f, translation.z + 100.0f,
                                 "%.2f" );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Scale" );

    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Xscale", &scale.x, 1.0f, 0.0f, 100.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Yscale", &scale.y, 1.0f, 0.0f, 100.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Zscale", &scale.z, 1.0f, 0.0f, 100.0f, "%.2f" );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Rotation" );

    auto& rotation = transformComponent.rotation;

    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Xrotation", &rotation.x, 0.1f, -180.0f, 180.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Yrotation", &rotation.y, 0.1f, -180.0f, 180.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Zrotation", &rotation.z, 0.1f, -180.0f, 180.0f, "%.2f" );

    ImGui::EndTable();

    if ( changed )
    {
      auto scene = SceneManager::getCurrentScene().lock();

      if ( !scene )
        return;

      // we need the index of the instance
      const auto& indexComponent = ent.getComponent<IndexComponent>();

      // get the instance data of this model
      const auto data = scene->getData( modelComponent->pMesh );

      // update the matrix in the instance matrices vector
      ImGuizmo::RecomposeMatrixFromComponents(
        glm::value_ptr( translation ), glm::value_ptr( rotation ), glm::value_ptr( scale ),
        glm::value_ptr( data->instances.at( indexComponent.index ).instanceMatrix ) );

      scene->updateInstances( data );
    }
  }
}

void EntityPropertiesWindow::drawPointLightComponent( kogayonon_core::Entity& ent ) const
{
  if ( !ent.hasComponent<PointLightComponent>() )
    return;
  ImGui::SeparatorText( "Point light" );
  const auto& pPointLightComponent = ent.getComponent<PointLightComponent>();

  auto scene = SceneManager::getCurrentScene().lock();

  if ( !scene )
    return;

  auto& pointLight = scene->getPointLight( pPointLightComponent.pointLightIndex );

  bool changed = false;

  static auto textSize = ImGui::CalcTextSize( "Translation" );

  if ( ImGui::BeginTable( "##table_pointLight", 4 ) )
  {
    ImGui::TableSetupColumn( "label", ImGuiTableColumnFlags_WidthFixed, textSize.x );
    ImGui::TableSetupColumn( "x", ImGuiTableColumnFlags_WidthFixed, 100.0f );
    ImGui::TableSetupColumn( "y", ImGuiTableColumnFlags_WidthFixed, 100.0f );
    ImGui::TableSetupColumn( "z", ImGuiTableColumnFlags_WidthFixed, 100.0f );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Translation" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Xtranslation", &pointLight.translation.x, 0.1f, pointLight.translation.x - 100.0f,
                                 pointLight.translation.x + 100.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Ytranslation", &pointLight.translation.y, 0.1f, pointLight.translation.y - 100.0f,
                                 pointLight.translation.y + 100.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Ztranslation", &pointLight.translation.z, 0.1f, pointLight.translation.z - 100.0f,
                                 pointLight.translation.z + 100.0f, "%.2f" );
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Distance" );
    ImGui::TableNextColumn();

    bool distance = false;
    static std::string distanceValue = "7";
    if ( ImGui::BeginCombo( "##distance", distanceValue.c_str() ) )
    {
      // x = constant, y = linear, z = quadratic, w = enabled

      if ( ImGui::MenuItem( "7" ) )
      {
        pointLight.params.y = 0.7f;
        pointLight.params.z = 1.8f;
        distance = true;
        distanceValue = "7";
      }
      if ( ImGui::MenuItem( "13" ) )
      {
        pointLight.params.y = 0.35f;
        pointLight.params.z = 0.44f;
        distance = true;
        distanceValue = "13";
      }
      if ( ImGui::MenuItem( "20" ) )
      {
        pointLight.params.y = 0.22f;
        pointLight.params.z = 0.20f;
        distance = true;
        distanceValue = "20";
      }
      if ( ImGui::MenuItem( "32" ) )
      {
        pointLight.params.y = 0.14f;
        pointLight.params.z = 0.07f;
        distance = true;
        distanceValue = "32";
      }
      if ( ImGui::MenuItem( "50" ) )
      {
        pointLight.params.y = 0.09f;
        pointLight.params.z = 0.032f;
        distance = true;
        distanceValue = "50";
      }
      if ( ImGui::MenuItem( "65" ) )
      {
        pointLight.params.y = 0.07f;
        pointLight.params.z = 0.017f;
        distance = true;
        distanceValue = "65";
      }
      if ( ImGui::MenuItem( "100" ) )
      {
        pointLight.params.y = 0.045f;
        pointLight.params.z = 0.0075f;
        distance = true;
        distanceValue = "100";
      }
      if ( ImGui::MenuItem( "160" ) )
      {
        pointLight.params.y = 0.027f;
        pointLight.params.z = 0.0028f;
        distance = true;
        distanceValue = "160";
      }
      if ( ImGui::MenuItem( "200" ) )
      {
        pointLight.params.y = 0.022f;
        pointLight.params.z = 0.0019f;
        distance = true;
        distanceValue = "200";
      }
      if ( ImGui::MenuItem( "325" ) )
      {
        pointLight.params.y = 0.014f;
        pointLight.params.z = 0.0007f;
        distance = true;
        distanceValue = "325";
      }
      if ( ImGui::MenuItem( "600" ) )
      {
        pointLight.params.y = 0.007f;
        pointLight.params.z = 0.0002f;
        distance = true;
        distanceValue = "600";
      }
      if ( ImGui::MenuItem( "3250" ) )
      {
        pointLight.params.y = 0.0014f;
        pointLight.params.z = 0.000007f;
        distance = true;
        distanceValue = "3250";
      }
      ImGui::EndCombo();
    }
    changed |= distance;

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Color" );
    ImGui::TableNextColumn();
    changed |= ImGui::ColorEdit4( "##color_change", reinterpret_cast<float*>( &pointLight.color ),
                                  ImGuiColorEditFlags_NoInputs );
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    bool visible = pointLight.params.w > 0.5f ? true : false;
    if ( ImGui::Checkbox( "##enable", &visible ) )
    {
      pointLight.params.w = visible;
      changed |= true;
    }

    ImGui::EndTable();

    if ( changed )
    {
      auto scene = SceneManager::getCurrentScene().lock();
      // update the ubo and ssbo if needed
      scene->updateLightBuffers();
    }
  }
}

void EntityPropertiesWindow::drawDirectionalLightComponent( kogayonon_core::Entity& ent ) const
{
  if ( !ent.hasComponent<DirectionalLightComponent>() )
    return;

  ImGui::SeparatorText( "Directional light" );
  const auto& pDirectionalLightComponent = ent.tryGetComponent<DirectionalLightComponent>();

  if ( !pDirectionalLightComponent )
    return;

  auto scene = SceneManager::getCurrentScene().lock();

  if ( !scene )
    return;

  auto& directionalLight = scene->getDirectionalLight( pDirectionalLightComponent->directionalLightIndex );

  bool changed = false;

  static auto textSize = ImGui::CalcTextSize( "Position factor" );

  if ( ImGui::BeginTable( "##table_directional_light", 4 ) )
  {
    ImGui::TableSetupColumn( "label", ImGuiTableColumnFlags_WidthFixed, textSize.x );
    ImGui::TableSetupColumn( "x", ImGuiTableColumnFlags_WidthFixed, 100.0f );
    ImGui::TableSetupColumn( "y", ImGuiTableColumnFlags_WidthFixed, 100.0f );
    ImGui::TableSetupColumn( "z", ImGuiTableColumnFlags_WidthFixed, 100.0f );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Direction" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Xdirection", &directionalLight.direction.x, 0.1f,
                                 directionalLight.direction.x - 100.0f, directionalLight.direction.x + 100.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Ydirection", &directionalLight.direction.y, 0.1f,
                                 directionalLight.direction.y - 100.0f, directionalLight.direction.y + 100.0f, "%.2f" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##Zdirection", &directionalLight.direction.z, 0.1f,
                                 directionalLight.direction.z - 100.0f, directionalLight.direction.z + 100.0f, "%.2f" );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Ortho size" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##OrthoSize", &pDirectionalLightComponent->orthoSize, 0.1f, 0.1f, 2000.0f, "%.2f" );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Near plane" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##NearPlane", &pDirectionalLightComponent->nearPlane, 0.1f, 0.1f, 2000.0f, "%.2f" );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Far plane" );
    ImGui::TableNextColumn();
    changed |= ImGui::DragFloat( "##FarPlane", &pDirectionalLightComponent->farPlane, 0.1f, 0.1f, 2000.0f, "%.2f" );

    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImGui::Text( "Position factor" );
    ImGui::TableNextColumn();
    changed |=
      ImGui::DragFloat( "##PositionFactor", &pDirectionalLightComponent->positionFactor, 0.1f, 0.1f, 2000.0f, "%.2f" );

    ImGui::EndTable();

    if ( changed )
    {
      auto scene = SceneManager::getCurrentScene().lock();
      // update the ubo and ssbo if needed
      scene->updateLightBuffers();
    }
  }
}
} // namespace kogayonon_gui