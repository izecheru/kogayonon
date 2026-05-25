#include "gui/imgui_windows/entity_properties.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "graphics/vulkan_context.hpp"
#include "gui/utils/font_keys.hpp"
#include "gui/utils/imgui_dragdrop_defines.hpp"
#include "gui/utils/imgui_utils.hpp"
#include "physics/jolt_physics.hpp"
#include "utilities/fonts/materialdesign.hpp"
#include "utilities/utils/utils.hpp"
#include <imgui_stdlib.h>

gui::EntityProperties::EntityProperties( const std::string& name, const EntityPropertiesSpec& spec )
    : ImGuiWindow{ name }
    , m_spec{ spec }
    , m_selectedEntity{ entt::null }
{
  auto& pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<core::SelectEntityEvent, &EntityProperties::onSelectEntity>( *this );
}

void gui::EntityProperties::render()
{
  if ( !begin() )
    return;

  if ( m_selectedEntity == entt::null )
  {
    ImGui::PushFont( m_spec.fonts->at( INTER_I ), 18.0f );
    ImGui::Text( "No entity selected..." );
    ImGui::PopFont();
    ImGui::End();
    return;
  }

  contextMenu();

  renderIdentification();
  renderTransform();
  renderMesh();
  renderCamera();

  ImGui::End();
}

void gui::EntityProperties::onSelectEntity( const core::SelectEntityEvent& e )
{
  // accept event from anywhere BUT itself
  if ( e.getEventSource() == core::SelectEntityEventSource::Properties_Window )
    return;

  if ( e.getEntityId() == entt::null && e.getEventSource() != core::SelectEntityEventSource::Properties_Window )
  {
    m_selectedEntity = entt::null;
    KOGAYONON_INFO( "entity was deselected, nothing to show in properties window" );
  }
  else
  {
    m_selectedEntity = e.getEntityId();
    KOGAYONON_INFO( "showing properties of entity with id {}", static_cast<uint32_t>( m_selectedEntity ) );
  }
}

void gui::EntityProperties::contextMenu()
{
  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  if ( ImGui::BeginCombo( "##", "Add component" ) )
  {
    auto scene = core::SceneManager::getCurrentScene().lock();
    auto meshComp = scene->getRegistry()->tryGetComponent<core::MeshComponent>( m_selectedEntity );
    bool hasCamera = scene->getRegistry()->hasComponent<core::PerspectiveCameraComponent>( m_selectedEntity );

    if ( !meshComp && !hasCamera )
    {
      if ( ImGui::MenuItem( "Mesh" ) )
      {
        auto p =
          std::filesystem::path{ std::filesystem::absolute( "." ) / "engine_resources\\models\\default_cone.gltf" };
        auto mesh = core::AssetManager::getInstance().getMesh( p.string() );
        if ( mesh.has_value() )
        {
          scene->getRegistry()->addComponent<core::TransformComponent>( m_selectedEntity, core::TransformComponent{} );
          scene->getRegistry()->addComponent<core::MeshComponent>( m_selectedEntity,
                                                                   core::MeshComponent{
                                                                     .pMesh = nullptr,
                                                                     .loaded = false,
                                                                   } );
        }
      }
    }
    else
    {
      RenderDisabled( ImGui::MenuItem( "Mesh" ) );
    }

    bool hasRigidBody = scene->getRegistry()->hasComponent<core::RigidbodyComponent>( m_selectedEntity );
    if ( !hasRigidBody && meshComp )
    {
      if ( ImGui::MenuItem( "Dynamic rigid body" ) )
      {
        auto& jolt = core::MainRegistry::getInstance().getJoltPhysics();
        auto& transform = scene->getRegistry()->getComponent<core::TransformComponent>( m_selectedEntity );
        scene->getRegistry()->addComponent<core::RigidbodyComponent>(
          m_selectedEntity,
          core::RigidbodyComponent{
            .type = physics::RigidbodyType::Dynamic,
            .body = jolt->createRigidBody(
              physics::RigidbodyType::Dynamic,
              physics::RigidbodyShape::Box,
              { transform.translation.x, transform.translation.y, transform.translation.z },
              { transform.scale.x,
                transform.scale.y,
                transform.scale.y }, // TODO(kogayonon) detemrine size somehow, with a bounding box i guess
              glm::quat{ glm::radians( transform.rotation ) } ) } );
      }
    }

    if ( !hasRigidBody && meshComp )
    {
      if ( ImGui::MenuItem( "Static rigid body" ) )
      {
        auto& jolt = core::MainRegistry::getInstance().getJoltPhysics();
        auto& transform = scene->getRegistry()->getComponent<core::TransformComponent>( m_selectedEntity );
        scene->getRegistry()->addComponent<core::RigidbodyComponent>(
          m_selectedEntity,
          core::RigidbodyComponent{
            .type = physics::RigidbodyType::Static,
            .body = jolt->createRigidBody(
              physics::RigidbodyType::Static,
              physics::RigidbodyShape::Box,
              { transform.translation.x, transform.translation.y, transform.translation.z },
              { transform.scale.x,
                transform.scale.y,
                transform.scale.y }, // TODO(kogayonon) detemrine size somehow, with a bounding box i guess
              glm::quat{ glm::radians( transform.rotation ) } ) } );
      }
    }

    ImGui::EndCombo();
  }
  ImGui::PopFont();
}

void gui::EntityProperties::renderMesh()
{
  if ( m_selectedEntity == entt::null )
    return;

  auto scene = core::SceneManager::getCurrentScene().lock();
  auto meshComponent = scene->getRegistry()->tryGetComponent<core::MeshComponent>( m_selectedEntity );

  if ( !meshComponent )
    return;

  gui_utils::renderWithFont( m_spec.fonts->at( INTER_I ), [&]() { ImGui::SeparatorText( "Mesh component" ); } );

  gui_utils::renderWithSizedFont( m_spec.fonts->at( ICON_MDI ), 12.0f, [&]() {
    if ( ImGui::Button( ICON_MDI_DELETE "Remove component" ) )
    {
      scene->getRegistry()->removeComponent<core::MeshComponent>( m_selectedEntity );
      scene->getRegistry()->removeComponent<core::TransformComponent>( m_selectedEntity );
    }
  } );

  if ( !meshComponent->pMesh )
  {
    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 14.0f, []() {
      ImGui::Text( "You don't have a loaded mesh at the moment, drop a model file here to load it" );
    } );

    if ( ImGui::BeginDragDropTarget() )
    {
      auto payload = ImGui::AcceptDragDropPayload( ASSET_DROP );

      if ( !payload )
      {
        return;
      }

      auto data = static_cast<const char*>( payload->Data );
      std::string dropResult( data, payload->DataSize );
      std::filesystem::path p{ dropResult };
      auto& assetManager = core::AssetManager::getInstance();
      auto pMesh = assetManager.loadMesh( p.stem().string(), p.string() );

      core::Entity ent{ scene->getRegistry(), m_selectedEntity };

      ent.removeComponent<core::TransformComponent>();
      ent.removeComponent<core::MeshComponent>();

      ent.addComponent<core::TransformComponent>( core::TransformComponent{} );
      ent.addComponent<core::MeshComponent>( core::MeshComponent{ .pMesh = pMesh, .loaded = true } );
    }
  }
  else
  {
    auto meshPath = std::filesystem::path{ meshComponent->pMesh->getPath() };
    ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
    ImGui::Text( "Filename: %s", meshPath.stem().string().c_str() );
    ImGui::PopFont();
  }
}

void gui::EntityProperties::renderCamera()
{
  if ( m_selectedEntity == entt::null )
    return;

  auto scene = core::SceneManager::getCurrentScene().lock();
  auto camera = scene->getRegistry()->tryGetComponent<core::PerspectiveCameraComponent>( m_selectedEntity );

  if ( !camera )
    return;

  gui_utils::renderWithFont( m_spec.fonts->at( INTER_I ), []() { ImGui::SeparatorText( "Camera component" ); } );
  // this is if we want to link the values and make them equal
  static bool translationLink{ false };
  static bool scaleLink{ false };
  static bool rotationLink{ false };

  gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::Text( "Translation" ); } );
  // render translation here
  if ( !translationLink )
  {
    renderTranslation( camera->props.changed, camera->props.eye );
  }
  else
  {
    RenderDisabled( renderTranslation( camera->props.changed, camera->props.eye ) );
    ImGui::SameLine();

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
      ImGui::PushItemWidth( 50.0f );
      camera->props.changed |= ImGui::DragFloat( "##transCameraTranslation", &camera->props.eye.x, 0.04f );
      ImGui::PopItemWidth();
    } );
    camera->props.eye.y = camera->props.eye.x;
    camera->props.eye.z = camera->props.eye.x;
  }
  ImGui::SameLine();
  gui_utils::renderWithSizedFont(
    m_spec.fonts->at( INTER ), 16.0f, [&]() { ImGui::Checkbox( ICON_MDI_LINK "##tLink", &translationLink ); } );

  renderCameraFov( camera->props.changed, camera->props.fov );
  renderCameraNear( camera->props.changed, camera->props.nearView );
  renderCameraFar( camera->props.changed, camera->props.farView );
}

void gui::EntityProperties::renderCameraFov( bool& changed, float& fov )
{
  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  changed |= ImGui::DragFloat( "FOV", &fov );
  ImGui::PopFont();
}

void gui::EntityProperties::renderCameraNear( bool& changed, float& camNear )
{
  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  changed |= ImGui::DragFloat( "Near", &camNear );
  ImGui::PopFont();
}

void gui::EntityProperties::renderCameraFar( bool& changed, float& camFar )
{
  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  changed |= ImGui::DragFloat( "Far", &camFar );
  ImGui::PopFont();
}

void gui::EntityProperties::renderTransform()
{
  if ( m_selectedEntity == entt::null )
    return;

  auto scene = core::SceneManager::getCurrentScene().lock();

  auto pTransform = scene->getRegistry()->tryGetComponent<core::TransformComponent>( m_selectedEntity );

  if ( !pTransform )
    return;

  gui_utils::renderWithFont( m_spec.fonts->at( INTER_I ), []() { ImGui::SeparatorText( "Transform component" ); } );
  // this is if we want to link the values and make them equal
  static bool translationLink{ false };
  static bool scaleLink{ false };
  static bool rotationLink{ false };

  bool translationChanged{ false };
  bool scaleChanged{ false };
  bool rotationChanged{ false };

  gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::Text( "Translation" ); } );
  // render translation here
  if ( !translationLink )
  {
    renderTranslation( translationChanged, pTransform->translation );
  }
  else
  {
    RenderDisabled( renderTranslation( translationChanged, pTransform->translation ) );
    ImGui::SameLine();

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
      ImGui::PushItemWidth( 50.0f );
      translationChanged |= ImGui::DragFloat( "##transTranslation", &pTransform->translation.x, 0.04f );
      ImGui::PopItemWidth();
    } );

    pTransform->translation.y = pTransform->translation.x;
    pTransform->translation.z = pTransform->translation.x;
  }
  ImGui::SameLine();
  gui_utils::renderWithSizedFont(
    m_spec.fonts->at( INTER ), 16.0f, [&]() { ImGui::Checkbox( ICON_MDI_LINK "##tLink", &translationLink ); } );

  gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::Text( "Scale" ); } );
  // render scale here
  if ( !scaleLink )
  {
    renderScale( scaleChanged, pTransform->scale );
  }
  else
  {
    RenderDisabled( renderScale( scaleChanged, pTransform->scale ) );
    ImGui::SameLine();

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
      ImGui::PushItemWidth( 50.0f );
      translationChanged |= ImGui::DragFloat( "##transScale", &pTransform->scale.x, 0.04f );
      ImGui::PopItemWidth();
    } );

    pTransform->scale.y = pTransform->scale.x;
    pTransform->scale.z = pTransform->scale.x;
  }
  ImGui::SameLine();

  gui_utils::renderWithSizedFont(
    m_spec.fonts->at( INTER ), 16.0f, [&]() { ImGui::Checkbox( ICON_MDI_LINK "##sLink", &scaleLink ); } );

  gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::Text( "Rotation" ); } );
  // render scale here
  if ( !rotationLink )
  {
    renderRotation( rotationChanged, pTransform->rotation );
  }
  else
  {
    RenderDisabled( renderRotation( rotationChanged, pTransform->rotation ) );
    ImGui::SameLine();

    gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 16.0f, [&]() {
      ImGui::PushItemWidth( 50.0f );
      translationChanged |= ImGui::DragFloat( "##transRotation", &pTransform->rotation.x, 0.04f );
      ImGui::PopItemWidth();
    } );

    pTransform->rotation.y = pTransform->rotation.x;
    pTransform->rotation.z = pTransform->rotation.x;
  }
  ImGui::SameLine();

  gui_utils::renderWithSizedFont(
    m_spec.fonts->at( INTER ), 16.0f, [&]() { ImGui::Checkbox( ICON_MDI_LINK "##rLink", &rotationLink ); } );

  if ( translationChanged || scaleChanged || rotationChanged )
  {
    pTransform->update = true;
  }
}

void gui::EntityProperties::renderTranslation( bool& translationChanged, glm::vec3& translation )
{
  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  ImGui::PushItemWidth( 50.0f );

  renderColoredAxis( "X##t", COL_RED_LA );
  ImGui::SameLine();

  translationChanged |= ImGui::DragFloat( "##transCameraX", &translation.x, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Y##t", COL_GREEN_LA );
  ImGui::SameLine();

  translationChanged |= ImGui::DragFloat( "##transCameraY", &translation.y, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Z##t", COL_BLUE_LA );
  ImGui::SameLine();

  translationChanged |= ImGui::DragFloat( "##transCameraZ", &translation.z, 0.04f );
  ImGui::PopFont();
  ImGui::PopItemWidth();
}

void gui::EntityProperties::renderScale( bool& scaleChanged, glm::vec3& scale )
{
  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  ImGui::PushItemWidth( 50.0f );

  renderColoredAxis( "X##s", COL_RED_LA );
  ImGui::SameLine();

  scaleChanged = ImGui::DragFloat( "##scaleCameraX", &scale.x, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Y##s", COL_GREEN_LA );
  ImGui::SameLine();

  scaleChanged |= ImGui::DragFloat( "##scaleCameraY", &scale.y, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Z##s", COL_BLUE_LA );
  ImGui::SameLine();

  scaleChanged |= ImGui::DragFloat( "##scaleCameraZ", &scale.z, 0.04f );

  ImGui::PopFont();
  ImGui::PopItemWidth();
}

void gui::EntityProperties::renderRotation( bool& rotationChanged, glm::vec3& rotation )
{
  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  ImGui::PushItemWidth( 50.0f );

  renderColoredAxis( "X##r", COL_RED_LA );
  ImGui::SameLine();

  rotationChanged = ImGui::DragFloat( "##rotationX", &rotation.x, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Y##r", COL_GREEN_LA );
  ImGui::SameLine();

  rotationChanged |= ImGui::DragFloat( "##rotationY", &rotation.y, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Z##r", COL_BLUE_LA );
  ImGui::SameLine();

  rotationChanged |= ImGui::DragFloat( "##rotationZ", &rotation.z, 0.04f );
  ImGui::PopFont();
  ImGui::PopItemWidth();
}

void gui::EntityProperties::renderIdentification()
{
  if ( m_selectedEntity == entt::null )
    return;

  auto scene = core::SceneManager::getCurrentScene().lock();
  ImGui::PushFont( m_spec.fonts->at( INTER_I ) );
  ImGui::SeparatorText( "Entity identification" );
  ImGui::PopFont();

  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  core::Entity selectedEntity{ scene->getRegistry(), m_selectedEntity };
  auto& idComp = selectedEntity.getComponent<core::IdentifierComponent>();
  ImGui::InputText( "##id", &idComp.name );
  ImGui::Text( "Group: %s", idComp.group.c_str() );
  ImGui::PopFont();
}

void gui::EntityProperties::renderColoredAxis( const std::string& axis, const ImU32& color )
{
  ImGui::PushStyleColor( ImGuiCol_Button, color );
  ImGui::PushStyleColor( ImGuiCol_ButtonActive, color );
  ImGui::PushStyleColor( ImGuiCol_ButtonHovered, color );
  ImGui::PushStyleColor( ImGuiCol_Border, COL_LIGHT_GRAY_LA );
  ImGui::Button( axis.c_str() );
  ImGui::PopStyleColor( 4 );
}
