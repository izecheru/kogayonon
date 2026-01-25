#include "gui/scene_viewport.hpp"
#include <ImGuizmo.h>
#include <bitset>
#include <filesystem>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include "core/ecs/components/directional_light_component.hpp"
#include "core/ecs/components/identifier_component.hpp"
#include "core/ecs/components/index_component.hpp"
#include "core/ecs/components/mesh_component.hpp"
#include "core/ecs/components/outline_component.hpp"
#include "core/ecs/components/rigidbody_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "core/systems/rendering_system.hpp"
#include "physics/nvidia_physx.hpp"
#include "rendering/camera/camera.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/shader/shader_manager.hpp"
#include "utilities/time_tracker/time_tracker.hpp"
#include "utilities/utils/utils.hpp"

using namespace kogayonon_core;
using namespace kogayonon_rendering;
using namespace kogayonon_utilities;

constexpr ImVec2 toolbarButtonSize{ 14.0f, 14.0f };

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

SceneViewportWindow::SceneViewportWindow( SDL_Window* mainWindow, std::string name )
    : ImGuiWindow{ name }
    , m_selectedEntity{ entt::null }
    , m_mainWindow{ mainWindow }
    , m_pRenderingSystem{ std::make_unique<RenderingSystem>() }
    , m_pCamera{ std::make_unique<Camera>() }
    , m_gizmoMode{ GizmoMode::TRANSLATE }
{
  // buffer where we draw everything, geometry, lights
  FramebufferSpec spec{
    { FramebufferAttachment{ .textureFormat = GL_RGBA8, .type = FramebufferAttachmentType::Color },
      FramebufferAttachment{ .textureFormat = GL_DEPTH_COMPONENT24, .type = FramebufferAttachmentType::Depth } } };

  // entity picking in the viewport
  FramebufferSpec pickingSpec{
    { FramebufferAttachment{ .textureFormat = GL_RED_INTEGER, .type = FramebufferAttachmentType::Color },
      FramebufferAttachment{ .textureFormat = GL_DEPTH_COMPONENT24, .type = FramebufferAttachmentType::Depth } } };

  // this is for shadow map and all depth related stuff, also used in outline pass
  FramebufferSpec depthSpec{
    { FramebufferAttachment{ .textureFormat = GL_DEPTH_COMPONENT24, .type = FramebufferAttachmentType::Depth } } };

  // outline of the entity selected
  FramebufferSpec outlineSpec{
    { FramebufferAttachment{ .textureFormat = GL_RGBA8, .type = FramebufferAttachmentType::Color },
      FramebufferAttachment{ .textureFormat = GL_DEPTH24_STENCIL8, .type = FramebufferAttachmentType::Depth } } };

  m_frameBuffer = OpenGLFramebuffer{ spec };
  m_pickingFrameBuffer = OpenGLFramebuffer{ pickingSpec };
  m_depthBuffer = OpenGLFramebuffer{ depthSpec };
  m_stencilBuffer = OpenGLFramebuffer{ outlineSpec };

  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<SelectEntityEvent, &SceneViewportWindow::onSelectedEntity>( *this );
  pEventDispatcher->addHandler<MouseMovedEvent, &SceneViewportWindow::onMouseMoved>( *this );
  pEventDispatcher->addHandler<MouseClickedEvent, &SceneViewportWindow::onMouseClicked>( *this );
  pEventDispatcher->addHandler<KeyPressedEvent, &SceneViewportWindow::onKeyPressed>( *this );
  pEventDispatcher->addHandler<MouseScrolledEvent, &SceneViewportWindow::onMouseScrolled>( *this );

  m_playTextureId = AssetManager::getInstance().getTexture( "play.png" ).lock()->getTextureId();
  m_stopTextureId = AssetManager::getInstance().getTexture( "stop.png" ).lock()->getTextureId();
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
  if ( m_selectedEntity == e.getEntityId() || e.getEventSource() == SelectEntityEventSource::ViewportWindow )
    return;

  m_selectedEntity = e.getEntityId();
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
    SDL_WarpMouseInWindow( m_mainWindow,
                           static_cast<int>( m_props->x + m_props->width / 2 ),
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
  const auto& scene = SceneManager::getCurrentScene().lock();
  if ( !scene )
    return;

  // prepare model entities for rendering if they were not loaded
  scene->prepareForRendering();

  const auto& pShaderManager = MainRegistry::getInstance().getShaderManager();
  auto proj = m_pCamera->getProjectionMatrix( { m_props->width, m_props->height } );
  auto& view = m_pCamera->getViewMatrix();
  auto& depthShader = pShaderManager->getShader( "depth" );
  auto& depthDebugShader = pShaderManager->getShader( "depthDebug" );
  auto& geometryShader = pShaderManager->getShader( "3d" );
  auto& outliningShader = pShaderManager->getShader( "outlining" );
  auto& normalShader = pShaderManager->getShader( "3d_normal" );

  if ( scene->getLightCount( kogayonon_resources::LightType::Directional ) != 0 )
  {
    // since atm we have only one directional light we get the index 0 light
    const auto& directionalLight = scene->getDirectionalLight();

    // careful when deleting the directional light entity
    static entt::entity ent = entt::null;
    if ( ent == entt::null )
    {
      for ( const auto& [entity, lightComponent] : scene->getEnttRegistry().view<DirectionalLightComponent>().each() )
      {
        ent = entity;
      }
    }

    const auto& directionalLightComponent = scene->getRegistry()->getComponent<DirectionalLightComponent>( ent );

    auto lightProjection = glm::ortho( -directionalLightComponent.orthoSize,
                                       directionalLightComponent.orthoSize,
                                       -directionalLightComponent.orthoSize,
                                       directionalLightComponent.orthoSize,
                                       directionalLightComponent.nearPlane,
                                       directionalLightComponent.farPlane );
    auto lightDir = glm::normalize(
      glm::vec3( directionalLight.direction.x, directionalLight.direction.y, directionalLight.direction.z ) );
    auto lightPos = -lightDir * directionalLightComponent.positionFactor;

    auto lightView = glm::lookAt( lightPos, glm::vec3{ 0.0f }, glm::vec3{ 0.0f, 1.0f, 0.0f } );

    auto lightSpaceMatrix = lightProjection * lightView;

    Canvas canvas{ .framebuffer = &m_depthBuffer,
                   .w = static_cast<int>( m_props->width ),
                   .h = static_cast<int>( m_props->height ) };

    FrameContext frameContext{
      .canvas = canvas, .scene = scene.get(), .view = &lightView, .projection = &lightProjection };

    DepthPassContext depthPass{ .shader = &depthShader };

    m_pRenderingSystem->renderDepthPass( frameContext, depthPass );

    auto depthMap = m_depthBuffer.getDepthAttachmentId();
    GeometryPassContext geometryPass{
      .shader = &geometryShader,
      .depthMap = &depthMap,
      .lightVP = &lightSpaceMatrix,
    };

    frameContext.canvas.framebuffer = &m_frameBuffer;
    frameContext.projection = &proj;
    frameContext.view = &view;

    m_pRenderingSystem->renderGeometryPass( frameContext, geometryPass );

    if ( m_selectedEntity != entt::null )
    {
      Entity entity{ scene->getRegistry(), m_selectedEntity };
      if ( entity.hasComponent<MeshComponent>() && entity.hasComponent<OutlineComponent>() )
      {
        frameContext.canvas.framebuffer = &m_stencilBuffer;

        OutliningPassContext outlinePass{
          .normalShader = &normalShader, .outlineShader = &outliningShader, .depthMap = &depthMap };

        m_pRenderingSystem->renderOutliningPass( frameContext, outlinePass );
      }
      else
      {
        scene->addOutline( m_selectedEntity );
      }
    }
  }
}

constexpr float topCornerMenu = 10.0f;

void SceneViewportWindow::drawPickingScene()
{
  // if we are editing the transform of an entity, don't deselect it to get another one
  if ( m_selectedEntity != entt::null && m_gizmoEnabled )
    return;

  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();

  const auto& io = ImGui::GetIO();
  auto [mx, my] = ImGui::GetMousePos();

  mx -= static_cast<float>( m_props->x );
  my -= static_cast<float>( m_props->y );

  if ( mx < topCornerMenu || my < topCornerMenu || mx > m_props->width || my > m_props->height )
    return;

  const auto& pShaderManager = MainRegistry::getInstance().getShaderManager();
  auto& shader = pShaderManager->getShader( "picking" );

  const auto& scene = SceneManager::getCurrentScene().lock().get();

  Canvas canvas{ .framebuffer = &m_pickingFrameBuffer,
                 .w = static_cast<int>( m_props->width ),
                 .h = static_cast<int>( m_props->height ) };

  FrameContext frameContext{ .canvas = canvas,
                             .scene = scene,
                             .view = &m_pCamera->getViewMatrix(),
                             .projection =
                               &m_pCamera->getProjectionMatrix( glm::vec2{ m_props->width, m_props->height } ) };

  PickingPassContext pickingPass{ .shader = &shader, .x = static_cast<int>( mx ), .y = static_cast<int>( my ) };

  auto result = m_pRenderingSystem->renderPickingPass( frameContext, pickingPass );

  auto ent = static_cast<entt::entity>( result );
  if ( scene->getRegistry()->isValid( ent ) )
  {
    scene->addOutline( ent );
    m_selectedEntity = ent;
    pEventDispatcher->dispatchEvent( SelectEntityEvent{ ent, SelectEntityEventSource::ViewportWindow } );
  }
}

void SceneViewportWindow::onKeyPressed( const KeyPressedEvent& e )
{
  // change gizmo mode if we press SHIFT + S R T and once selected SHIFT + X Y Z for axis

  if ( !KeyboardState::getKeyState( KeyScanCode::LeftShift ) )
    return;

  switch ( e.getKeyScanCode() )
  {
  case KeyScanCode::G:
    m_gizmoEnabled = !m_gizmoEnabled;
    break;
  case KeyScanCode::S:
    m_gizmoMode = GizmoMode::SCALE;
    break;
  case KeyScanCode::R:
    m_gizmoMode = GizmoMode::ROTATE;
    break;
  case KeyScanCode::T:
    m_gizmoMode = GizmoMode::TRANSLATE;
    break;

  case KeyScanCode::X:
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

  case KeyScanCode::Y:
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

  case KeyScanCode::Z:
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

  auto& assetManager = AssetManager::getInstance();

  // this must be called every frame, not on key press function because it would never move smoothly
  // also the keyboard button check is a map of keys with a bool set to true if button is currently pressed
  if ( m_props->focused && !KeyboardState::getKeyState( KeyScanCode::LeftShift ) )
  {
    const auto& pTimeTracker = MainRegistry::getInstance().getTimeTracker();
    m_pCamera->onKeyPressed( static_cast<float>( pTimeTracker->getDuration( "deltaTime" ).count() ) );
  }

  const auto& scene = SceneManager::getCurrentScene().lock();
  ImVec2 win_pos = ImGui::GetCursorScreenPos();
  ImVec2 contentSize = ImGui::GetContentRegionAvail();

  scene->updateRigidbodyEntities();

  drawScene();

  ImGui::GetWindowDrawList()->AddImage( m_frameBuffer.getColorAttachmentId( 0 ),
                                        win_pos,
                                        ImVec2{ win_pos.x + contentSize.x, win_pos.y + contentSize.y },
                                        ImVec2{ 0, 1 },
                                        ImVec2{ 1, 0 } );

  if ( m_selectedEntity != entt::null && scene->getRegistry()->hasComponent<MeshComponent>( m_selectedEntity ) )
  {
    const auto& meshComp = scene->getRegistry()->getComponent<MeshComponent>( m_selectedEntity );
    if ( meshComp.loaded && meshComp.pMesh != nullptr )
    {
      ImGui::GetWindowDrawList()->AddImage( m_stencilBuffer.getColorAttachmentId( 0 ),
                                            win_pos,
                                            ImVec2{ win_pos.x + contentSize.x, win_pos.y + contentSize.y },
                                            ImVec2{ 0, 1 },
                                            ImVec2{ 1, 0 } );
    }
  }

  ImGuizmo::SetOrthographic( false );
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect( win_pos.x, win_pos.y, contentSize.x, contentSize.y );

  if ( m_selectedEntity != entt::null && m_gizmoEnabled )
  {
    Entity entity{ scene->getRegistry(), m_selectedEntity };
    const auto& modelComponent = entity.tryGetComponent<MeshComponent>();
    if ( modelComponent && modelComponent->pMesh != nullptr && modelComponent->loaded == true )
    {
      const auto& instanceData = scene->getData( modelComponent->pMesh );
      const auto& indexComponet = entity.tryGetComponent<IndexComponent>();
      const auto& transform = entity.tryGetComponent<TransformComponent>();
      auto& instanceMatrix = instanceData->instances.at( indexComponet->index ).instanceMatrix;

      ImGuizmo::Enable( ( m_props->hovered && m_props->focused ) || ImGuizmo::IsUsingAny() );

      ImGuizmo::Manipulate( glm::value_ptr( m_pCamera->getViewMatrix() ),
                            glm::value_ptr( m_pCamera->getProjectionMatrix( { contentSize.x, contentSize.y } ) ),
                            gizmoModeToImGuizmo( m_gizmoMode ),
                            ImGuizmo::WORLD,
                            glm::value_ptr( instanceMatrix ) );

      if ( ImGuizmo::IsUsing() )
      {
        glm::vec3 translation, rotation, scale;
        ImGuizmo::DecomposeMatrixToComponents( glm::value_ptr( instanceMatrix ),
                                               glm::value_ptr( translation ),
                                               glm::value_ptr( rotation ),
                                               glm::value_ptr( scale ) );

        transform->translation = translation;
        transform->rotation = rotation;
        transform->scale = scale;
        scene->updateInstances( instanceData );

        auto quat = glm::quat{ glm::radians( transform->rotation ) };

        // now that translation has been changed, need to update physics too
        if ( entity.hasComponent<DynamicRigidbodyComponent>() )
        {
          auto dynamicRigidBody = entity.getComponent<DynamicRigidbodyComponent>();
          dynamicRigidBody.pBody->setGlobalPose(
            physx::PxTransform{ transform->translation.x,
                                transform->translation.y,
                                transform->translation.z,
                                physx::PxQuat{ quat.x, quat.y, quat.z, quat.w } } );
        }
        else if ( entity.hasComponent<StaticRigidbodyComponent>() )
        {
          auto staticRigidBody = entity.getComponent<StaticRigidbodyComponent>();
          staticRigidBody.pBody->setGlobalPose( physx::PxTransform{ transform->translation.x,
                                                                    transform->translation.y,
                                                                    transform->translation.z,
                                                                    physx::PxQuat{ quat.x, quat.y, quat.z, quat.w } } );
        }
      }
    }
  }

  drawToolbar();

  bool open = true;
  static auto padding = ImVec2{ 10.0f, 10.0f };
  ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, padding );

  if ( ImGui::BeginPopupModal( "Render mode", &open, ImGuiWindowFlags_AlwaysAutoResize ) )
  {
    ImGui::Text( "You can change the render modes here, this is a work in progress though" );

    if ( ImGui::Button( "Geometry alone" ) )
    {
      m_renderMode = RenderMode::Geometry;
      ImGui::CloseCurrentPopup();
      open = false;
    }

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

void SceneViewportWindow::drawToolbar()
{
  auto& style = ImGui::GetStyle();
  auto& assetManager = AssetManager::getInstance();

  static auto renderModeIcon = assetManager.getTexture( "render_mode_icon.png" ).lock()->getTextureId();
  static auto playIcon = assetManager.getTexture( "play.png" ).lock()->getTextureId();
  static auto stopIcon = assetManager.getTexture( "stop.png" ).lock()->getTextureId();

  // WARNING thake this into account when mouse picking
  ImGui::SetCursorPos( ImVec2{ topCornerMenu, topCornerMenu } );

  ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 8, 8 ) );
  ImGui::PushStyleVar( ImGuiStyleVar_ChildRounding, 10.0f );
  ImGui::PushStyleColor( ImGuiCol_ChildBg, ImVec4{ 0.15f, 0.15f, 0.15f, 0.75f } );

  constexpr int buttonCount = 3;
  static float toolbarWidth = style.WindowPadding.x * 2.0f + ( toolbarButtonSize.x * buttonCount ) +
                              ( style.ItemSpacing.x * buttonCount ) + ( 2.0f * 5.0f );

  if ( ImGui::BeginChild( "Toolbar",
                          ImVec2{ toolbarWidth, 25.0f },
                          false,
                          ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse ) )
  {
    ImGui::PushStyleColor( ImGuiCol_Button, ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f } );

    ImGui::SetCursorPos( ImVec2{ 5.0f, 2.5f } );
    if ( ImGui::ImageButton( "#RenderMode", renderModeIcon, toolbarButtonSize ) )
    {
    }
    ImGui::SameLine();

    if ( ImGui::ImageButton( "#StopButton", stopIcon, toolbarButtonSize ) )
    {
      kogayonon_physics::NvidiaPhysx::getInstance().switchState( false );
    }
    ImGui::SameLine();

    if ( ImGui::ImageButton( "#StartButton", playIcon, toolbarButtonSize ) )
    {
      kogayonon_physics::NvidiaPhysx::getInstance().switchState( true );
    }

    ImGui::PopStyleColor();
    ImGui::EndChild();
  }

  ImGui::PopStyleColor();
  ImGui::PopStyleVar( 2 );
}
} // namespace kogayonon_gui