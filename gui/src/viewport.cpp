#include "gui/imgui_windows/viewport.hpp"
#include "core/asset_manager/asset_manager.hpp"
#include "core/ecs/components/camera_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "gui/utils/font_keys.hpp"
#include "gui/utils/imgui_utils.hpp"
#include "physics/jolt_physics.hpp"
#include "utilities/fonts/materialdesign.hpp"
#include "utilities/input/keyboard_state.hpp"
#include "utilities/utils/utils.hpp"
#include <ImOGuizmo.hpp>
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>

gui::Viewport::Viewport( SDL_Window* mainWindow, const std::string& name, const ViewportSpec& spec )
    : ImGuiWindow{ name, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar }
    , m_spec{ spec }
    , m_mainWindow{ mainWindow }
    , m_guizmoMode{ GuizmoMode::SCALE }
    , m_guizmoAxisLock{ AxisLock::NONE }
    , m_selectedEntity{ entt::null }
    , m_guizmoOp{ ImGuizmo::SCALE }
    , m_guizmoEnabled{ false }
    , m_entityMenu{ false }
    , m_viewportDescriptor{ VK_NULL_HANDLE }
    , m_mouseCoords{ 0.0f, 0.0f }
{
  auto& pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<core::SelectEntityEvent, &Viewport::onEntitySelect>( *this );
  pEventDispatcher->addHandler<core::KeyPressedEvent, &Viewport::onKeyPressed>( *this );
  // pEventDispatcher->addHandler<core::MouseClickedEvent, &Viewport::onMouseClicked>( *this );
}

void gui::Viewport::render()
{
  ImGuiWindowClass windowClass;
  windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

  ImGui::SetNextWindowClass( &windowClass );

  if ( !begin() )
    return;

  auto viewportPanelSize = ImGui::GetContentRegionAvail();
  static bool first{ true };

  if ( first )
  {
    m_viewportDescriptor =
      ImGui_ImplVulkan_AddTexture( m_spec.sampler, m_spec.viewportTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
    first = false;
  }

  // For the click to go to the guizmo
  ImGui::SetNextItemAllowOverlap();
  ImGui::Image( m_viewportDescriptor, viewportPanelSize );
  auto min = ImGui::GetItemRectMin();
  auto max = ImGui::GetItemRectMax();

  drawToolbar();
  drawEntityMenu();

  auto scene = core::SceneManager::getCurrentScene().lock();
  auto view = scene->getEnttRegistry().view<core::PerspectiveCameraComponent>();
  view.each( [&]( const entt::entity& entityId, core::PerspectiveCameraComponent& cameraComp ) {
    if ( cameraComp.isUsed )
    {
      ImOGuizmo::SetDrawList( ImGui::GetWindowDrawList() );
      ImOGuizmo::SetRect( max.x - 110.0f, min.y + 10.0f, 100.0f );
      gui_utils::renderWithSizedFont( m_spec.fonts->at( INTER ), 12.0f, [&]() {
        if ( ImOGuizmo::DrawGizmo(
               glm::value_ptr( cameraComp.ubo.view ), glm::value_ptr( cameraComp.ubo.projection ), 0.1f ) )
        {
        }
      } );
    }
  } );

  auto& jolt = core::MainRegistry::getInstance().getJoltPhysics();
  if ( m_selectedEntity != entt::null && m_guizmoEnabled && !jolt->isRunning() )
  {
    ImGuizmo::Enable( true );
    auto transformComp = scene->getRegistry()->tryGetComponent<core::TransformComponent>( m_selectedEntity );
    if ( transformComp )
    {
      ImGuizmo::SetOrthographic( false );
      ImGuizmo::SetDrawlist( ImGui::GetWindowDrawList() );

      ImGuizmo::SetRect( min.x, min.y, max.x - min.x, max.y - min.y );

      auto view = scene->getEnttRegistry().view<core::PerspectiveCameraComponent>();
      entt::entity cameraEntity = entt::null;
      view.each( [&]( const entt::entity& entityId, core::PerspectiveCameraComponent& cameraComp ) {
        if ( cameraComp.isUsed )
        {
          cameraEntity = entityId;
        }
      } );

      auto& cameraComponent = scene->getRegistry()->getComponent<core::PerspectiveCameraComponent>( cameraEntity );
      auto projection = cameraComponent.ubo.projection;

      // Unflip the projection
      projection[1][1] *= -1;
      ImGuizmo::Manipulate( glm::value_ptr( cameraComponent.ubo.view ),
                            glm::value_ptr( projection ),
                            getGuizmoOp(),
                            ImGuizmo::LOCAL,
                            glm::value_ptr( transformComp->getMatrix() ) );

      if ( ImGuizmo::IsUsing() )
      {
        ImGuizmo::DecomposeMatrixToComponents( glm::value_ptr( transformComp->getMatrix() ),
                                               glm::value_ptr( transformComp->translation ),
                                               glm::value_ptr( transformComp->rotation ),
                                               glm::value_ptr( transformComp->scale ) );

        // If the entity has a rigid body then update the position and rotation of that too
        if ( auto pBody = scene->getRegistry()->tryGetComponent<core::RigidbodyComponent>( m_selectedEntity ) )
        {
          // Now set position and rotation for the rigid body
          auto& bodyInterface = jolt->getPhysicsSystem().GetBodyInterface();
          auto quat = transformComp->getOrientation();
          bodyInterface.SetPositionAndRotation(
            pBody->body,
            { transformComp->translation.x, transformComp->translation.y, transformComp->translation.z },
            JPH::Quat{ quat.x, quat.y, quat.z, quat.w },
            pBody->data.activation );
        }
      }
    }
  }
  ImGui::End();
}

void gui::Viewport::drawToolbar()
{
  auto& style = ImGui::GetStyle();

  ImGui::SetCursorPos( { 20.0f, 20.0f } );

  ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, { 8.0f, 8.0f } );
  ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 10.0f );
  ImGui::PushStyleColor( ImGuiCol_ChildBg, { 0.15f, 0.15f, 0.15f, 0.75f } );

  uint32_t buttonCount = m_guizmoEnabled ? 5u : 2u;
  float toolbarWidth =
    style.WindowPadding.x * 2.0f + ( 14.0f * buttonCount ) + ( style.ItemSpacing.x * buttonCount ) + ( 2.0f * 5.0f );

  ImGui::BeginGroup();
  if ( ImGui::BeginChild( "Toolbar",
                          { toolbarWidth, 30.0f },
                          false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
  {

    ImGui::PushStyleColor( ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f } );
    ImGui::PushStyleColor( ImGuiCol_Border, { 0.0f, 0.0f, 0.0f, 0.0f } );

    ImGui::SetCursorPos( { 5.0f, 5.5f } );
    auto& jolt = core::MainRegistry::getInstance().getJoltPhysics();

    if ( ImGui::ImageButton( "##stopButton", m_spec.stopIcon, { 14.0f, 14.0f } ) )
    {
      jolt->stop();
    }
    ImGui::SameLine();

    if ( ImGui::ImageButton( "##startButton", m_spec.playIcon, { 14.0f, 14.0f } ) )
    {
      jolt->start();
    }

    ImGui::SameLine();
    if ( m_guizmoEnabled )
    {
      if ( m_guizmoAxisLock == AxisLock::X )
      {
        gui_utils::renderWithSizedFont(
          m_spec.fonts->at( INTER ), 12.0f, []() { ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, "X" ); } );
      }
      else
      {
        gui_utils::renderWithSizedFont(
          m_spec.fonts->at( INTER ), 12.0f, []() { ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 0.3f }, "X" ); } );
      }
      ImGui::SameLine();
      if ( m_guizmoAxisLock == AxisLock::Y )
      {
        gui_utils::renderWithSizedFont(
          m_spec.fonts->at( INTER ), 12.0f, []() { ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, "Y" ); } );
      }
      else
      {
        gui_utils::renderWithSizedFont(
          m_spec.fonts->at( INTER ), 12.0f, []() { ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 0.3f }, "Y" ); } );
      }
      ImGui::SameLine();
      if ( m_guizmoAxisLock == AxisLock::Z )
      {
        gui_utils::renderWithSizedFont(
          m_spec.fonts->at( INTER ), 12.0f, []() { ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f }, "Z" ); } );
      }
      else
      {
        gui_utils::renderWithSizedFont(
          m_spec.fonts->at( INTER ), 12.0f, []() { ImGui::TextColored( ImVec4{ 1.0f, 1.0f, 1.0f, 0.3f }, "Z" ); } );
      }
    }
  }

  ImGui::PopStyleColor( 2 );

  ImGui::EndChild();
  ImGui::EndGroup();

  ImGui::PopStyleColor();
  ImGui::PopStyleVar( 2 );
}

void gui::Viewport::drawEntityMenu()
{
  if ( !m_entityMenu )
    return;

  if ( m_mouseCoords.x == 0.0f && m_mouseCoords.y == 0.0f )
  {
    auto mouse = ImGui::GetMousePos();
    if ( m_props->hovered )
    {
      m_mouseCoords = { mouse.x, mouse.y };
    }
    else
    {
      auto size = ImGui::GetWindowSize();
      m_mouseCoords = { m_props->x + ( size.x * 0.5f ) - 130.0f, m_props->y + ( size.y * 0.5f ) - 100.0f };
    }
  }

  ImGui::SetNextWindowSize( { 130.0f, 190.0f } );
  ImGui::SetNextWindowPos( { m_mouseCoords.x, m_mouseCoords.y } );

  ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, { 8.0f, 8.0f } );
  ImGui::PushStyleVar( ImGuiStyleVar_FramePadding, { 8.0f, 8.0f } );
  ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 10.0f );
  ImGui::PushStyleColor( ImGuiCol_ChildBg, { 0.15f, 0.15f, 0.15f, 0.75f } );

  if ( ImGui::Begin(
         "##quickMenu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove ) )
  {
    // Close the menu if we the mouse does not hover over the window but we detect a click
    if ( !ImGui::IsWindowHovered( ImGuiHoveredFlags_RootAndChildWindows ) &&
         ImGui::IsMouseClicked( ImGuiMouseButton_Left ) )
    {
      m_entityMenu = false;
      m_mouseCoords = { 0.0f, 0.0f };
    }

    ImGui::PushItemWidth( 130.0f / 2.0f );

    auto scene = core::SceneManager::getCurrentScene().lock();
    auto& pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
    auto& assetManager = core::AssetManager::getInstance();

    ImGui::PushFont( m_spec.fonts->at( INTER ), 14.0f );
    if ( ImGui::BeginMenu( "Add object" ) )
    {
      std::string filename{ "" };
      bool selected{ false };

      if ( ImGui::MenuItem( "Cone" ) )
      {
        filename = "default_cone";
        selected = true;
      }

      if ( ImGui::MenuItem( "Cube" ) )
      {
        filename = "cub";
        selected = true;
      }

      if ( ImGui::MenuItem( "Sphere" ) )
      {
        filename = "sphere";
        selected = true;
      }

      if ( ImGui::MenuItem( "Ico Sphere" ) )
      {
        filename = "ico_sphere";
        selected = true;
      }

      if ( ImGui::MenuItem( "Cylinder" ) )
      {
        filename = "cylinder";
        selected = true;
      }

      if ( ImGui::MenuItem( "Torus" ) )
      {
        filename = "torus";
        selected = true;
      }

      if ( ImGui::MenuItem( "Plane" ) )
      {
        filename = "plane";
        selected = true;
      }

      if ( selected )
      {
        filename += ".gltf";
        std::filesystem::path p{ std::filesystem::absolute( "." ) / "engine_resources" / "models" / filename };

        core::Entity ent{ scene->getRegistry(), "object" };

        ent.addComponent<core::TransformComponent>( core::TransformComponent{} );
        ent.addComponent<core::MeshComponent>(
          core::MeshComponent{ .pMesh = assetManager.loadMesh( "test", p.string() ), .loaded = true } );

        pEventDispatcher->dispatchEvent<core::SelectEntityEvent>(
          core::SelectEntityEvent{ ent.getEntityId(), core::SelectEntityEventSource::Viewport_Window } );

        m_selectedEntity = ent.getEntityId();

        m_entityMenu = false;
        m_mouseCoords = { 0.0f, 0.0f };
        KOGAYONON_INFO( "menu closed" );
      }
      ImGui::EndMenu();
    }
    if ( m_selectedEntity != entt::null )
    {
      if ( ImGui::BeginMenu( "Add component" ) )
      {
        ImGui::EndMenu();
      }
    }
    else
    {
      RenderDisabled( ImGui::MenuItem( "Add component" ) );
    }

    ImGui::PopFont();

    ImGui::PopItemWidth();
  }
  ImGui::End();

  ImGui::PopStyleColor();
  ImGui::PopStyleVar( 3 );
}

auto gui::Viewport::getGuizmoOp() -> ImGuizmo::OPERATION
{
  switch ( m_guizmoMode )
  {
  case GuizmoMode::SCALE: {
    switch ( m_guizmoAxisLock )
    {
    case AxisLock::NONE:
      return ImGuizmo::SCALE;
    case AxisLock::X:
      return ImGuizmo::SCALE_X;
    case AxisLock::Y:
      return ImGuizmo::SCALE_Y;
    case AxisLock::Z:
      return ImGuizmo::SCALE_Z;
    }
    break;
  }

  case GuizmoMode::ROTATE: {
    switch ( m_guizmoAxisLock )
    {
    case AxisLock::NONE:
      return ImGuizmo::ROTATE;
    case AxisLock::X:
      return ImGuizmo::ROTATE_X;
    case AxisLock::Y:
      return ImGuizmo::ROTATE_Y;
    case AxisLock::Z:
      return ImGuizmo::ROTATE_Z;
    }
    break;
  }

  case GuizmoMode::TRANSLATE: {
    switch ( m_guizmoAxisLock )
    {
    case AxisLock::NONE:
      return ImGuizmo::TRANSLATE;
    case AxisLock::X:
      return ImGuizmo::TRANSLATE_X;
    case AxisLock::Y:
      return ImGuizmo::TRANSLATE_Y;
    case AxisLock::Z:
      return ImGuizmo::TRANSLATE_Z;
    }
    break;
  }
  }
}

void gui::Viewport::onEntitySelect( const core::SelectEntityEvent& e )
{
  if ( e.getEntityId() == m_selectedEntity )
    return;

  if ( e.getEntityId() == entt::null && e.getEventSource() == core::SelectEntityEventSource::None )
  {
    m_selectedEntity = entt::null;
    return;
  }

  m_selectedEntity = e.getEntityId();
}

void gui::Viewport::onKeyPressed( const core::KeyPressedEvent& e )
{
  auto& pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
  if ( KeyboardState::getKeyCombinationState( { KeyScanCode::LeftShift, KeyScanCode::G } ) )
  {
    m_guizmoEnabled = !m_guizmoEnabled;
  }

  if ( KeyboardState::getKeyCombinationState( { KeyScanCode::LeftShift, KeyScanCode::A } ) )
  {
    if ( m_entityMenu )
      m_mouseCoords = { 0.0f, 0.0f }; // Reset mouse coords on close

    m_entityMenu = !m_entityMenu;
  }

  if ( KeyboardState::getKeyState( KeyScanCode::Escape ) )
  {
    pEventDispatcher->dispatchEvent<core::SelectEntityEvent>( core::SelectEntityEvent{} );
    m_selectedEntity = entt::null;
  }

  // Guizmo op down
  if ( !m_guizmoEnabled )
    return;

  if ( e.getKeyModifier() == KeyScanCode::LeftShift )
  {
    switch ( e.getKeyScanCode() )
    {
    case KeyScanCode::T: {
      m_guizmoMode = GuizmoMode::TRANSLATE;
      break;
    }
    case KeyScanCode::R: {
      m_guizmoMode = GuizmoMode::ROTATE;
      break;
    }
    case KeyScanCode::S: {
      m_guizmoMode = GuizmoMode::SCALE;
      break;
    }

    case KeyScanCode::X: {
      m_guizmoAxisLock = ( m_guizmoAxisLock == AxisLock::X ) ? AxisLock::NONE : AxisLock::X;
      break;
    }
    case KeyScanCode::Y: {
      m_guizmoAxisLock = ( m_guizmoAxisLock == AxisLock::Y ) ? AxisLock::NONE : AxisLock::Y;
      break;
    }
    case KeyScanCode::Z: {
      m_guizmoAxisLock = ( m_guizmoAxisLock == AxisLock::Z ) ? AxisLock::NONE : AxisLock::Z;
      break;
    }
    }
  }
}
