#include "gui/scene_viewport.hpp"
#include <ImGuizmo.h>
#include <filesystem>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>
#include <spdlog/spdlog.h>
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "core/systems/rendering_system.h"
#include "rendering/camera/camera.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/serializer/serializer.hpp"
#include "utilities/shader_manager/shader_manager.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include "utilities/time_tracker/time_tracker.hpp"

using namespace kogayonon_core;
using namespace kogayonon_rendering;
using namespace kogayonon_utilities;

namespace kogayonon_gui
{
static ImGuizmo::OPERATION gizmoModeToImGuizmo( GizmoMode mode )
{
  switch ( mode )
  {
  case GizmoMode::SCALE:
    return ImGuizmo::SCALE;
  case GizmoMode::SCALE_X:
    return ImGuizmo::SCALE_X;
  case GizmoMode::SCALE_Y:
    return ImGuizmo::SCALE_Y;
  case GizmoMode::SCALE_Z:
    return ImGuizmo::SCALE_Z;

  case GizmoMode::ROTATE:
    return ImGuizmo::ROTATE;
  case GizmoMode::ROTATE_X:
    return ImGuizmo::ROTATE_X;
  case GizmoMode::ROTATE_Y:
    return ImGuizmo::ROTATE_Y;
  case GizmoMode::ROTATE_Z:
    return ImGuizmo::ROTATE_Z;

  case GizmoMode::TRANSLATE:
    return ImGuizmo::TRANSLATE;
  case GizmoMode::TRANSLATE_X:
    return ImGuizmo::TRANSLATE_X;
  case GizmoMode::TRANSLATE_Y:
    return ImGuizmo::TRANSLATE_Y;
  case GizmoMode::TRANSLATE_Z:
    return ImGuizmo::TRANSLATE_Z;
  }
}

SceneViewportWindow::SceneViewportWindow( SDL_Window* mainWindow, std::string name, unsigned int playTextureId,
                                          unsigned int stopTextureId )
    : ImGuiWindow{ name }
    , m_selectedEntity{ entt::null }
    , m_playTextureId{ playTextureId }
    , m_stopTextureId{ stopTextureId }
    , m_mainWindow{ mainWindow }
    , m_pRenderingSystem{ std::make_unique<RenderingSystem>() }
    , m_pCamera{ std::make_unique<Camera>() }
    , m_gizmoMode{ GizmoMode::TRANSLATE }
{
  FramebufferSpecification spec{
    { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH24STENCIL8 }, 800, 800 };
  FramebufferSpecification pickingSpec{
    { FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::DEPTH24STENCIL8 }, 800, 800 };

  FramebufferSpecification depthSpec{ { FramebufferTextureFormat::RGBA, FramebufferTextureFormat::DEPTH }, 800, 800 };

  m_frameBuffer = OpenGLFramebuffer{ spec };
  m_pickingFrameBuffer = OpenGLFramebuffer{ pickingSpec };
  m_depthBuffer = OpenGLFramebuffer{ depthSpec };

  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<SelectEntityEvent, &SceneViewportWindow::onSelectedEntity>( *this );
  pEventDispatcher->addHandler<MouseMovedEvent, &SceneViewportWindow::onMouseMoved>( *this );
  pEventDispatcher->addHandler<MouseClickedEvent, &SceneViewportWindow::onMouseClicked>( *this );
  pEventDispatcher->addHandler<KeyPressedEvent, &SceneViewportWindow::onKeyPressed>( *this );
  pEventDispatcher->addHandler<MouseScrolledEvent, &SceneViewportWindow::onMouseScrolled>( *this );
}

void SceneViewportWindow::onSaveScene( const SaveSceneEvent& e )
{
}

void SceneViewportWindow::onMouseScrolled( const MouseScrolledEvent& e )
{
  if ( !m_props->hovered )
    return;

  auto yOffset = static_cast<float>( e.getYOff() );
  m_pCamera->zoom( yOffset );
}

void SceneViewportWindow::onSelectedEntity( const SelectEntityEvent& e )
{
  if ( m_selectedEntity == e.getEntity() )
    return;

  m_selectedEntity = e.getEntity();
}

void SceneViewportWindow::onMouseMoved( const MouseMovedEvent& e )
{
  if ( !m_props->hovered )
    return;

  const auto& io = ImGui::GetIO();
  if ( io.MouseDown[ImGuiMouseButton_Right] )
  {
    SDL_SetRelativeMouseMode( SDL_TRUE );
    auto x = static_cast<float>( e.getXRel() );
    auto y = static_cast<float>( e.getYRel() );
    m_pCamera->onMouseMoved( x, y, true );

    // move mouse in the center
    SDL_WarpMouseInWindow( m_mainWindow, static_cast<int>( m_props->x + m_props->width / 2 ),
                           static_cast<int>( m_props->y + m_props->height / 2 ) );
  }
  else
  {
    SDL_SetRelativeMouseMode( SDL_FALSE );
  }
}

void SceneViewportWindow::onMouseClicked( const MouseClickedEvent& e )
{
  if ( e.getButton() != MouseCode::BUTTON_1 )
    return;

  drawPickingScene();
}

void SceneViewportWindow::drawScene()
{
  // prepare model entities for rendering if they were not loaded
  auto scene = SceneManager::getCurrentScene().lock();

  scene->prepareForRendering();

  const auto& pShaderManager = MainRegistry::getInstance().getShaderManager();
  const auto& size = ImGui::GetWindowSize();
  glm::mat4 proj = m_pCamera->getProjectionMatrix( { size.x, size.y } );

  auto& depthShader = pShaderManager->getShader( "depth" );
  auto& geometryShader = pShaderManager->getShader( "3d" );
  switch ( m_renderMode )
  {
  case RenderMode::GeometryAndLights:
    if ( scene )
    {
      auto& spec = m_frameBuffer.getSpecification();
      m_frameBuffer.resize( static_cast<int>( m_props->width ), static_cast<int>( m_props->height ) );
      m_frameBuffer.bind();
      glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      scene->bindLightBuffers();
      m_pRenderingSystem->render( scene, m_pCamera->getViewMatrix(), proj, geometryShader );
      scene->unbindLightBuffers();
      m_frameBuffer.unbind();
    }
    break;
  case RenderMode::Geometry:
    break;
  case RenderMode::Depth:
    if ( scene )
    {
      auto& spec = m_frameBuffer.getSpecification();
      m_depthBuffer.resize( static_cast<int>( m_props->width ), static_cast<int>( m_props->height ) );
      m_depthBuffer.bind();
      glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
      glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

      m_pRenderingSystem->render( scene, m_pCamera->getViewMatrix(), proj, depthShader );
      m_depthBuffer.unbind();
    }
    break;
  }
}

void SceneViewportWindow::drawPickingScene()
{
  // you must deselect the current entity to be able to select a new one
  if ( m_selectedEntity != entt::null )
    return;

  const auto& io = ImGui::GetIO();
  auto [mx, my] = ImGui::GetMousePos();

  mx -= static_cast<float>( m_props->x );
  my -= static_cast<float>( m_props->y );

  if ( mx < 0 || my < 0 || mx > m_props->width || my > m_props->height )
    return;

  const auto& pShaderManager = MainRegistry::getInstance().getShaderManager();
  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  auto& shader = pShaderManager->getShader( "picking" );
  m_pickingFrameBuffer.resize( m_props->width, m_props->height );

  auto proj = m_pCamera->getProjectionMatrix( { m_props->width, m_props->height } );
  m_pickingFrameBuffer.bind();
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  m_pickingFrameBuffer.clearColorAttachment( 0, -1 );
  m_pRenderingSystem->render( SceneManager::getCurrentScene().lock(), m_pCamera->getViewMatrix(), proj, shader );
  int result = m_pickingFrameBuffer.readPixel( 0, static_cast<int>( mx ), static_cast<int>( my ) );
  m_pickingFrameBuffer.unbind();

  if ( const auto& scene = SceneManager::getCurrentScene().lock() )
  {
    auto ent = static_cast<entt::entity>( result );
    if ( scene->getRegistry().isValid( ent ) )
    {
      // select
      if ( m_selectedEntity == ent )
        return;

      m_selectedEntity = ent;
      pEventDispatcher->emitEvent( SelectEntityInViewportEvent{ ent } );
    }
  }
}

void SceneViewportWindow::onKeyPressed( const KeyPressedEvent& e )
{
  // change gizmo mode if we press SHIFT + S R T
  if ( !KeyboardState::getKeyState( KeyCode::LeftShift ) )
    return;

  switch ( e.getKeyCode() )
  {
  case KeyCode::S:
    m_gizmoMode = GizmoMode::SCALE;
    break;
  case KeyCode::R:
    m_gizmoMode = GizmoMode::ROTATE;
    break;
  case KeyCode::T:
    m_gizmoMode = GizmoMode::TRANSLATE;
    break;

  case KeyCode::X:
    if ( m_gizmoMode == GizmoMode::SCALE || m_gizmoMode == GizmoMode::SCALE_Y || m_gizmoMode == GizmoMode::SCALE_Z )
    {
      m_gizmoMode = GizmoMode::SCALE_X;
      break;
    }

    if ( m_gizmoMode == GizmoMode::TRANSLATE || m_gizmoMode == GizmoMode::TRANSLATE_Y ||
         m_gizmoMode == GizmoMode::TRANSLATE_Z )
    {
      m_gizmoMode = GizmoMode::TRANSLATE_X;
      break;
    }

    if ( m_gizmoMode == GizmoMode::ROTATE || m_gizmoMode == GizmoMode::ROTATE_Y || m_gizmoMode == GizmoMode::ROTATE_Z )
      m_gizmoMode = GizmoMode::ROTATE_X;
    break;

  case KeyCode::Y:
    if ( m_gizmoMode == GizmoMode::SCALE || m_gizmoMode == GizmoMode::SCALE_X || m_gizmoMode == GizmoMode::SCALE_Z )
    {
      m_gizmoMode = GizmoMode::SCALE_Y;
      break;
    }

    if ( m_gizmoMode == GizmoMode::TRANSLATE || m_gizmoMode == GizmoMode::TRANSLATE_X ||
         m_gizmoMode == GizmoMode::TRANSLATE_Z )
    {
      m_gizmoMode = GizmoMode::TRANSLATE_Y;
      break;
    }

    if ( m_gizmoMode == GizmoMode::ROTATE || m_gizmoMode == GizmoMode::ROTATE_X || m_gizmoMode == GizmoMode::ROTATE_Z )
      m_gizmoMode = GizmoMode::ROTATE_Y;

    break;

  case KeyCode::Z:
    if ( m_gizmoMode == GizmoMode::SCALE || m_gizmoMode == GizmoMode::SCALE_X || m_gizmoMode == GizmoMode::SCALE_Y )
    {
      m_gizmoMode = GizmoMode::SCALE_Z;
      break;
    }

    if ( m_gizmoMode == GizmoMode::TRANSLATE || m_gizmoMode == GizmoMode::TRANSLATE_X ||
         m_gizmoMode == GizmoMode::TRANSLATE_Y )
    {
      m_gizmoMode = GizmoMode::TRANSLATE_Z;
      break;
    }

    if ( m_gizmoMode == GizmoMode::ROTATE || m_gizmoMode == GizmoMode::ROTATE_X || m_gizmoMode == GizmoMode::ROTATE_Y )
      m_gizmoMode = GizmoMode::ROTATE_Z;

    break;
  }
}

void SceneViewportWindow::draw()
{
  if ( !begin() )
    return;

  // this must be called every frame, not on key press function because it would never move smoothly
  // also the keyboard button check is a map of keys with a bool set to true if button is currently pressed
  if ( m_props->focused && !KeyboardState::getKeyState( KeyCode::LeftShift ) )
  {
    const auto& pTimeTracker = MainRegistry::getInstance().getTimeTracker();
    m_pCamera->onKeyPressed( static_cast<float>( pTimeTracker->getDuration( "deltaTime" ).count() ) );
  }

  const auto& scene = SceneManager::getCurrentScene().lock();
  ImVec2 win_pos = ImGui::GetCursorScreenPos();
  ImVec2 contentSize = ImGui::GetContentRegionAvail();

  drawScene();

  switch ( m_renderMode )
  {
  case RenderMode::Geometry:
  case RenderMode::GeometryAndLights:
    ImGui::GetWindowDrawList()->AddImage( m_frameBuffer.getColorAttachmentId( 0 ), win_pos,
                                          ImVec2{ win_pos.x + contentSize.x, win_pos.y + contentSize.y },
                                          ImVec2{ 0, 1 }, ImVec2{ 1, 0 } );
    break;
  default:
    ImGui::GetWindowDrawList()->AddImage( m_depthBuffer.getColorAttachmentId( 0 ), win_pos,
                                          ImVec2{ win_pos.x + contentSize.x, win_pos.y + contentSize.y },
                                          ImVec2{ 0, 1 }, ImVec2{ 1, 0 } );
  }

  ImGuizmo::SetOrthographic( false );
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect( win_pos.x, win_pos.y, contentSize.x, contentSize.y );

  if ( m_selectedEntity != entt::null )
  {
    auto& registry = scene->getRegistry();
    const auto& modelComponent = registry.tryGetComponent<MeshComponent>( m_selectedEntity );
    if ( modelComponent && modelComponent->pMesh != nullptr && modelComponent->loaded == true )
    {
      const auto& instanceData = scene->getData( modelComponent->pMesh );
      const auto& indexComponet = registry.tryGetComponent<IndexComponent>( m_selectedEntity );
      const auto& transform = registry.tryGetComponent<TransformComponent>( m_selectedEntity );
      auto& instanceMatrix = instanceData->instanceMatrices.at( indexComponet->index );

      ImGuizmo::Enable( ( m_props->hovered && m_props->focused ) || ImGuizmo::IsUsingAny() );

      ImGuizmo::Manipulate( glm::value_ptr( m_pCamera->getViewMatrix() ),
                            glm::value_ptr( m_pCamera->getProjectionMatrix( { contentSize.x, contentSize.y } ) ),
                            gizmoModeToImGuizmo( m_gizmoMode ), ImGuizmo::WORLD, glm::value_ptr( instanceMatrix ) );

      if ( ImGuizmo::IsUsing() )
      {
        glm::vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents( glm::value_ptr( instanceMatrix ), glm::value_ptr( translation ),
                                               glm::value_ptr( rotation ), glm::value_ptr( scale ) );

        transform->translation = translation;
        transform->rotation = rotation;
        transform->scale = scale;
        scene->setupMultipleInstances( instanceData );
      }
    }
  }

  const auto& pAssetManager = MainRegistry::getInstance().getAssetManager();
  static auto renderModeIcon = pAssetManager->getTextureByName( "render_mode_icon.png" ).lock()->getTextureId();
  ImGui::SetCursorPos( ImVec2{ 20.0f, 20.0f } );
  ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );       // normal
  ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImVec4( 0, 0, 0, 0 ) ); // active (pressed)

  // place the viewport buttons here
  if ( ImGui::ImageButton( "##RenderModeButton", renderModeIcon, ImVec2{ 20.0f, 20.0f }, ImVec2{ 0, 0 }, ImVec2{ 1, 1 },
                           ImVec4{ 0, 0, 0, 0 }, ImVec4{ 1, 1, 1, 1 } ) )
  {
    ImGui::OpenPopup( "Render mode" );
  }

  ImGui::PopStyleColor( 2 ); // pop Button, Hovered, Active

  bool open = true;
  static auto padding = ImVec2{ 10.0f, 10.0f };
  ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, padding );

  if ( ImGui::BeginPopupModal( "Render mode", &open, ImGuiWindowFlags_AlwaysAutoResize ) )
  {
    ImGui::Text( "You can change the render modes here, this is a work in progress though" );

    if ( ImGui::Button( "Geometry and lights" ) )
    {
      m_renderMode = RenderMode::GeometryAndLights;
      ImGui::CloseCurrentPopup();
      open = false;
    }

    if ( ImGui::Button( "Depth" ) )
    {
      m_renderMode = RenderMode::Depth;
      ImGui::CloseCurrentPopup();
      open = false;
    }

    ImGui::EndPopup();
  }
  ImGui::PopStyleVar();
  end();
}
} // namespace kogayonon_gui