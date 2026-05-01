#include "gui/imgui_windows/entity_properties.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "gui/utils/font_keys.hpp"
#include "gui/utils/imgui_utils.hpp"
#include "utilities/fonts/materialdesign.hpp"
#include "utilities/utils/utils.hpp"

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
  }

  renderIdentification();
  renderTransform();

  ImGui::End();
}

void gui::EntityProperties::onSelectEntity( const core::SelectEntityEvent& e )
{
  // accept event from anywhere BUT itself
  if ( e.getEventSource() == core::SelectEntityEventSource::PropertiesWindow )
    return;

  if ( e.getEntityId() == entt::null && e.getEventSource() != core::SelectEntityEventSource::PropertiesWindow )
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

  gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::LabelText( "##", "Translation" ); } );
  // render translation here
  if ( !translationLink )
  {
    renderTransformTranslation( translationChanged, pTransform );
  }
  else
  {
    RenderDisabled( renderTransformTranslation( translationChanged, pTransform ) );
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

  gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::LabelText( "##", "Scale" ); } );
  // render scale here
  if ( !scaleLink )
  {
    renderTransformScale( scaleChanged, pTransform );
  }
  else
  {
    RenderDisabled( renderTransformScale( scaleChanged, pTransform ) );
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

  gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 18.0f, []() { ImGui::LabelText( "##", "Rotation" ); } );
  // render scale here
  if ( !rotationLink )
  {
    renderTransformRotation( rotationChanged, pTransform );
  }
  else
  {
    RenderDisabled( renderTransformRotation( rotationChanged, pTransform ) );
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

void gui::EntityProperties::renderTransformTranslation( bool& translationChanged, core::TransformComponent* pTransform )
{
  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  ImGui::PushItemWidth( 50.0f );

  renderColoredAxis( "X##t", COL_RED_LA );
  ImGui::SameLine();

  translationChanged |= ImGui::DragFloat( "##transX", &pTransform->translation.x, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Y##t", COL_GREEN_LA );
  ImGui::SameLine();

  translationChanged |= ImGui::DragFloat( "##transY", &pTransform->translation.y, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Z##t", COL_BLUE_LA );
  ImGui::SameLine();

  translationChanged |= ImGui::DragFloat( "##transZ", &pTransform->translation.z, 0.04f );
  ImGui::PopFont();
  ImGui::PopItemWidth();
}

void gui::EntityProperties::renderTransformScale( bool& scaleChanged, core::TransformComponent* pTransform )
{
  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  ImGui::PushItemWidth( 50.0f );

  renderColoredAxis( "X##s", COL_RED_LA );
  ImGui::SameLine();

  scaleChanged = ImGui::DragFloat( "##scaleX", &pTransform->scale.x, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Y##s", COL_GREEN_LA );
  ImGui::SameLine();

  scaleChanged |= ImGui::DragFloat( "##scaleY", &pTransform->scale.y, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Z##s", COL_BLUE_LA );
  ImGui::SameLine();

  scaleChanged |= ImGui::DragFloat( "##scaleZ", &pTransform->scale.z, 0.04f );

  ImGui::PopFont();
  ImGui::PopItemWidth();
}

void gui::EntityProperties::renderTransformRotation( bool& rotationChanged, core::TransformComponent* pTransform )
{
  ImGui::PushFont( m_spec.fonts->at( INTER ), 16.0f );
  ImGui::PushItemWidth( 50.0f );

  renderColoredAxis( "X##r", COL_RED_LA );
  ImGui::SameLine();

  rotationChanged = ImGui::DragFloat( "##rotationX", &pTransform->rotation.x, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Y##r", COL_GREEN_LA );
  ImGui::SameLine();

  rotationChanged |= ImGui::DragFloat( "##rotationY", &pTransform->rotation.y, 0.04f );
  ImGui::SameLine();

  renderColoredAxis( "Z##r", COL_BLUE_LA );
  ImGui::SameLine();

  rotationChanged |= ImGui::DragFloat( "##rotationZ", &pTransform->rotation.z, 0.04f );
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
  ImGui::Text( "Name- %s", idComp.name.c_str() );
  ImGui::Text( "Group- %s", idComp.group.c_str() );
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
