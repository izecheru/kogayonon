#include "gui/scene_viewport.hpp"
#include <ImGuizmo.h>
#include <filesystem>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
#include "core/ecs/components/index_component.h"
#include "core/ecs/components/model_component.hpp"
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
  case GizmoMode::ROTATE:
    return ImGuizmo::ROTATE;
  case GizmoMode::TRANSLATE:
    return ImGuizmo::TRANSLATE;
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
  FramebufferSpecification spec{ { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::DEPTH }, 800, 800 };
  FramebufferSpecification pickingSpec{
    { FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::DEPTH }, 800, 800 };
  m_frameBuffer = OpenGLFramebuffer{ spec };
  m_pickingFrameBuffer = OpenGLFramebuffer{ pickingSpec };

  const auto& pEventDispatcher = MainRegistry::getInstance().getEventDispatcher();
  pEventDispatcher->addHandler<SelectEntityEvent, &SceneViewportWindow::onSelectedEntity>( *this );
  pEventDispatcher->addHandler<MouseMovedEvent, &SceneViewportWindow::onMouseMoved>( *this );
  pEventDispatcher->addHandler<MouseClickedEvent, &SceneViewportWindow::onMouseClicked>( *this );
  pEventDispatcher->addHandler<KeyPressedEvent, &SceneViewportWindow::onKeyPressed>( *this );
  pEventDispatcher->addHandler<MouseScrolledEvent, &SceneViewportWindow::onMouseScrolled>( *this );
}

void SceneViewportWindow::onSaveScene( const SaveSceneEvent& e )
{
  std::filesystem::path path{ std::filesystem::absolute( "resources\\scenes" ) };
  if ( !std::filesystem::exists( path ) )
  {
    std::filesystem::create_directories( path );

    auto scene = SceneManager::getCurrentScene().lock();
    auto scenePath = path.string() + "/" + scene->getName() + ".kscene";

    std::fstream out{ scenePath, std::ios::out | std::ios::binary };

    if ( !out )
      spdlog::error( "Could not create {}", scenePath );

    std::string test{ "test2" };
    size_t size = test.size();
    Serializer::serialize( size, out );
    Serializer::serialize( test.data(), test.size() * sizeof( char ), out );

    if ( out )
    {
      out.close();
    }
  }
  else
  {
    std::filesystem::create_directories( path );

    auto scene = SceneManager::getCurrentScene().lock();
    auto scenePath = path.string() + "/" + scene->getName() + ".kscene";

    std::fstream in{ scenePath, std::ios::in | std::ios::binary };

    size_t size;
    Serializer::deserialize( size, in );

    std::string test;
    test.resize( size );
    Serializer::deserialize( test.data(), size * sizeof( char ), in );
    spdlog::info( test );

    if ( in )
      in.close();
  }
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
  const auto& pAssetManager = MainRegistry::getInstance().getAssetManager();
  for ( const auto& [entity, modelComponent] : scene->getRegistry().getRegistry().view<ModelComponent>().each() )
  {
    if ( modelComponent.pModel == nullptr )
      continue;

    if ( modelComponent.loaded )
      continue;

    // if we don't have the model in the instance vector, we upload the geometry
    if ( !scene->addInstanceData( entity ) )
      pAssetManager->uploadMeshGeometry( modelComponent.pModel->getMeshes() );

    modelComponent.loaded = true;
  }

  auto& spec = m_frameBuffer.getSpecification();
  m_frameBuffer.resize( static_cast<int>( m_props->width ), static_cast<int>( m_props->height ) );
  m_frameBuffer.bind();
  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  const auto& pShaderManager = MainRegistry::getInstance().getShaderManager();
  auto& shader = pShaderManager->getShader( "3d" );
  const auto& size = ImGui::GetWindowSize();
  glm::mat4 proj = m_pCamera->getProjectionMatrix( { size.x, size.y } );

  if ( const auto& scene = SceneManager::getCurrentScene().lock() )
    m_pRenderingSystem->render( scene, m_pCamera->getViewMatrix(), proj, shader );

  m_frameBuffer.unbind();
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
  if ( KeyboardState::getKeyCombinationState( { KeyCode::LeftControl, KeyCode::S } ) )
  {
    onSaveScene( SaveSceneEvent{} );
  }

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

  ImGui::GetWindowDrawList()->AddImage( (ImTextureID)m_frameBuffer.getColorAttachmentId( 0 ), win_pos,
                                        ImVec2{ win_pos.x + contentSize.x, win_pos.y + contentSize.y }, ImVec2{ 0, 1 },
                                        ImVec2{ 1, 0 } );

  ImGuizmo::SetOrthographic( false );
  ImGuizmo::SetDrawlist();
  ImGuizmo::SetRect( win_pos.x, win_pos.y, contentSize.x, contentSize.y );

  if ( m_selectedEntity != entt::null )
  {
    Entity ent{ scene->getRegistry(), m_selectedEntity };
    const auto& modelComponent = ent.tryGetComponent<ModelComponent>();
    if ( modelComponent && modelComponent->pModel != nullptr && modelComponent->loaded == true )
    {
      const auto& instanceData = scene->getData( modelComponent->pModel );
      const auto& indexComponet = ent.tryGetComponent<IndexComponent>();
      const auto& transform = ent.tryGetComponent<TransformComponent>();
      auto& instanceMatrix = instanceData->instanceMatrices.at( indexComponet->index );

      ImGuizmo::Enable( ( m_props->hovered && m_props->focused ) || ImGuizmo::IsUsingAny() );

      ImGuizmo::Manipulate( glm::value_ptr( m_pCamera->getViewMatrix() ),
                            glm::value_ptr( m_pCamera->getProjectionMatrix( { contentSize.x, contentSize.y } ) ),
                            gizmoModeToImGuizmo( m_gizmoMode ), ImGuizmo::WORLD, glm::value_ptr( instanceMatrix ) );

      if ( ImGuizmo::IsUsing() )
      {
        ImGui::SetMouseCursor( ImGuiMouseCursor_None );
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

  end();
}
} // namespace kogayonon_gui