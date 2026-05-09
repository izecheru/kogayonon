#include "gui/imgui_windows/viewport.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "gui/utils/imgui_utils.hpp"
#include "utilities/fonts/materialdesign.hpp"
#include "utilities/input/keyboard_state.hpp"
#include "utilities/utils/utils.hpp"
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_internal.h>

gui::Viewport::Viewport( SDL_Window* mainWindow, const std::string& name, const ViewportSpec& spec )
    : ImGuiWindow{ name, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar }
    , m_spec{ spec }
    , m_mainWindow{ mainWindow }
    , m_guizmoMode{ GuizmoMode::ROTATE }
    , m_guizmoAxisLock{ AxisLock::NONE }
    , m_selectedEntity{ entt::null }
    , m_guizmoOp{ ImGuizmo::SCALE }
    , m_guizmoEnabled{ false }
{
  auto& pEventDispatcher = core::MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<core::SelectEntityEvent, &Viewport::onSelectedEntity>( *this );
  pEventDispatcher->addHandler<core::KeyPressedEvent, &Viewport::onKeyPressed>( *this );
}

void gui::Viewport::render()
{
  ImGuiWindowClass windowClass;
  windowClass.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoTabBar;

  ImGui::SetNextWindowClass( &windowClass );

  if ( !begin() )
    return;

  // TODO(kogayonon) draw the scene here
  auto viewportPanelSize = ImGui::GetContentRegionAvail();

  ImGui::BeginGroup();
  static auto viewportDescriptorSet =
    ImGui_ImplVulkan_AddTexture( *m_spec.pSampler, *m_spec.pViewportTexture, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
  ImGui::Image( viewportDescriptorSet, viewportPanelSize );

  drawToolbar();
  ImGui::EndGroup();

  if ( m_selectedEntity != entt::null && m_guizmoEnabled )
  {
    ImGuizmo::Enable( true );
    auto scene = core::SceneManager::getCurrentScene().lock();
    auto transformComp = scene->getRegistry()->tryGetComponent<core::TransformComponent>( m_selectedEntity );
    if ( transformComp )
    {
      ImGuizmo::SetOrthographic( false );
      ImGuizmo::SetDrawlist();

      auto windowPos = ImGui::GetWindowPos();
      auto windowSize = ImGui::GetWindowSize();

      auto view =
        glm::lookAt( glm::vec3{ 8.0f, 8.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f } );
      auto proj = glm::perspective( glm::radians( 50.0f ), windowSize.x / windowSize.y, 0.1f, 1000.0f );
      // proj[1][1] *= -1;

      ImGuizmo::SetRect( windowPos.x, windowPos.y, windowSize.x, windowSize.y );
      if ( ImGuizmo::Manipulate( glm::value_ptr( view ),
                                 glm::value_ptr( proj ),
                                 getGuizmoOp(),
                                 ImGuizmo::LOCAL,
                                 glm::value_ptr( transformComp->getMatrix() ) ) )
      {
        ImGuizmo::DecomposeMatrixToComponents( glm::value_ptr( transformComp->getMatrix() ),
                                               glm::value_ptr( transformComp->translation ),
                                               glm::value_ptr( transformComp->rotation ),
                                               glm::value_ptr( transformComp->scale ) );
      }
    }
  }

  ImGui::End();
}

void gui::Viewport::drawToolbar()
{
  auto& style = ImGui::GetStyle();

  // WARNING thake this into account when mouse picking
  ImGui::SetCursorPos( { 20.0f, 20.0f } );

  ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, { 8.0f, 8.0f } );
  ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 10.0f );
  ImGui::PushStyleColor( ImGuiCol_ChildBg, { 0.15f, 0.15f, 0.15f, 0.75f } );

  constexpr int buttonCount = 3;
  static float toolbarWidth =
    style.WindowPadding.x * 2.0f + ( 14.0f * buttonCount ) + ( style.ItemSpacing.x * buttonCount ) + ( 2.0f * 5.0f );

  if ( ImGui::BeginChild( "Toolbar",
                          { toolbarWidth, 25.0f },
                          false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
  {

    ImGui::PushStyleColor( ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f } );
    ImGui::PushStyleColor( ImGuiCol_Border, { 0.0f, 0.0f, 0.0f, 0.0f } );

    ImGui::SetCursorPos( { 5.0f, 2.5f } );
    if ( ImGui::ImageButton( "##renderMode", m_spec.renderModeIcon, { 14.0f, 14.0f } ) )
    {
    }
    ImGui::SameLine();

    if ( ImGui::ImageButton( "##stopButton", m_spec.stopIcon, { 14.0f, 14.0f } ) )
    {
      // kogayonon_physics::NvidiaPhysx::getInstance().switchState( false );
    }
    ImGui::SameLine();

    if ( ImGui::ImageButton( "##startButton", m_spec.playIcon, { 14.0f, 14.0f } ) )
    {
      // kogayonon_physics::NvidiaPhysx::getInstance().switchState( true );
    }

    ImGui::PopStyleColor( 2 );
    ImGui::EndChild();
  }

  ImGui::PopStyleColor();
  ImGui::PopStyleVar( 2 );
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

void gui::Viewport::onSelectedEntity( const core::SelectEntityEvent& e )
{
  if ( e.getEntityId() == m_selectedEntity || e.getEntityId() == entt::null )
    return;

  KOGAYONON_INFO( "selected entity in viewport received" );
  m_selectedEntity = e.getEntityId();
}

void gui::Viewport::onKeyPressed( const core::KeyPressedEvent& e )
{
  if ( KeyboardState::getKeyCombinationState( { KeyScanCode::LeftShift, KeyScanCode::G } ) )
  {
    m_guizmoEnabled = !m_guizmoEnabled;
  }

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
