#include "gui/scene_viewport.hpp"
#include <ImGuizmo.h>
#include <filesystem>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <spdlog/spdlog.h>
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
#include "utilities/shader_manager/shader_manager.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include "utilities/time_tracker/time_tracker.hpp"

using namespace kogayonon_core;
using namespace kogayonon_rendering;

namespace kogayonon_gui
{
SceneViewportWindow::SceneViewportWindow( SDL_Window* mainWindow, std::string name, unsigned int playTextureId,
                                          unsigned int stopTextureId )
    : ImGuiWindow{ std ::move( name ) }
    , m_selectedEntity{ entt::null }
    , m_playTextureId{ playTextureId }
    , m_stopTextureId{ stopTextureId }
    , m_mainWindow{ mainWindow }
    , m_pRenderingSystem{ std::make_unique<RenderingSystem>() }
    , m_pCamera{ std::make_unique<Camera>() }
{
  FramebufferSpecification spec{ { FramebufferTextureFormat::RGBA8 }, 300, 300 };
  FramebufferSpecification pickingSpec{ { FramebufferTextureFormat::RED_INTEGER }, 300, 300 };
  m_frameBuffer = OpenGLFramebuffer{ spec };
  m_pickingFrameBuffer = OpenGLFramebuffer{ pickingSpec };

  EVENT_DISPATCHER()->addHandler<SelectEntityEvent, &SceneViewportWindow::onSelectedEntity>( *this );
  EVENT_DISPATCHER()->addHandler<MouseMovedEvent, &SceneViewportWindow::onMouseMoved>( *this );
  EVENT_DISPATCHER()->addHandler<MouseClickedEvent, &SceneViewportWindow::onMouseClicked>( *this );
  EVENT_DISPATCHER()->addHandler<KeyPressedEvent, &SceneViewportWindow::onKeyPressed>( *this );
  EVENT_DISPATCHER()->addHandler<MouseScrolledEvent, &SceneViewportWindow::onMouseScrolled>( *this );
}

void SceneViewportWindow::onMouseScrolled( const MouseScrolledEvent& e )
{
  if ( !m_props || !m_props->hovered )
    return;

  auto yOffset = static_cast<float>( e.getYOff() );
  m_pCamera->zoom( yOffset );
}

void SceneViewportWindow::onSelectedEntity( const SelectEntityEvent& e )
{
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

struct Ray
{
  glm::vec3 Origin;
  glm::vec3 Direction;
};

void SceneViewportWindow::traceRay()
{
  auto [mx, my] = ImGui::GetMousePos();

  mx -= static_cast<float>( m_props->x );
  my -= static_cast<float>( m_props->y );

  float width = m_props->width;
  float height = m_props->height;
  float x = ( 2.0f * mx ) / width - 1.0f;
  float y = 1.0f - ( 2.0f * my ) / height;

  if ( mx < 0 || my < 0 || mx > width || my > height )
    return;

  int closestEntityIndex = -1;
  auto scene = SceneManager::getCurrentScene().lock();
  float closestDistance = std::numeric_limits<float>::max();

  glm::vec4 rayClip{ x, y, -1.0f, 1.0f };

  // Ray in eye space
  glm::vec4 rayEye = glm::inverse( m_pCamera->getProjectionMatrix( { width, height } ) ) * rayClip;
  rayEye = glm::vec4( rayEye.x, rayEye.y, -1.0f, 0.0f );

  // Ray in world space
  glm::vec3 rayWorld = glm::normalize( glm::vec3( glm::inverse( m_pCamera->getViewMatrix() ) * rayEye ) );
  Ray ray{ .Origin = m_pCamera->getPosition(), .Direction = rayWorld };
  for ( const auto& [entity, transform] : scene->getEnttRegistry().view<TransformComponent>().each() )
  {
    auto firstMax = std::max( transform.scale.x, transform.scale.y );
    float radius = 2.0f * std::max( firstMax, transform.scale.z );

    glm::vec3 toObject = ray.Origin - transform.pos;

    float a = glm::dot( ray.Direction, ray.Direction );
    float b = 2.0f * glm::dot( toObject, ray.Direction );
    float c = glm::dot( toObject, toObject ) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    if ( discriminant < 0.0f )
      continue;

    float t = ( -b - glm::sqrt( discriminant ) ) / ( 2.0f * a );
    if ( t > 0.0f && t < closestDistance )
    {
      closestDistance = t;
      closestEntityIndex = (int)entity;
    }
  }
  if ( closestEntityIndex >= 0 )
  {
    // select
    m_selectedEntity = static_cast<entt::entity>( closestEntityIndex );

    // if the entity is valid
    if ( scene->getRegistry().isValid( m_selectedEntity ) )
    {
      kogayonon_core::SelectEntityInViewportEvent e{ m_selectedEntity };
      TASK_MANAGER()->enqueue( [&e]() { EVENT_DISPATCHER()->emitEvent( e ); } );
      return;
    }
  }

  // deselect
  if ( m_selectedEntity != entt::null )
  {
    kogayonon_core::SelectEntityInViewportEvent e{};
    TASK_MANAGER()->enqueue( [&e]() { EVENT_DISPATCHER()->emitEvent( e ); } );
  }
}

void SceneViewportWindow::onMouseClicked( const MouseClickedEvent& e )
{
}

void SceneViewportWindow::drawScene( ImVec2 viewportPos )
{
  auto& spec = m_frameBuffer.getSpecification();
  m_frameBuffer.bind();
  glDrawBuffer( GL_COLOR_ATTACHMENT0 );

  m_frameBuffer.resize( static_cast<int>( m_props->width ), static_cast<int>( m_props->height ) );
  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  auto& shader = SHADER_MANAGER()->getShader( "3d" );
  const auto& size = ImGui::GetWindowSize();
  glm::mat4 proj = m_pCamera->getProjectionMatrix( { size.x, size.y } );
  m_pRenderingSystem->render( SceneManager::getCurrentScene().lock(), m_pCamera->getViewMatrix(), proj, shader );
  m_frameBuffer.unbind();
}

void SceneViewportWindow::drawPickingScene( ImVec2 viewportPos )
{
  const auto& io = ImGui::GetIO();

  // if ( !io.MouseDown[ImGuiMouseButton_Left] )
  //   return;
  auto [mx, my] = ImGui::GetMousePos();

  mx -= static_cast<float>( m_props->x );
  my -= static_cast<float>( m_props->y );

  float width = m_props->width;
  float height = m_props->height;

  if ( mx < 0 || my < 0 || mx > width || my > height )
    return;

  auto& shader = SHADER_MANAGER()->getShader( "3d" );
  m_pickingFrameBuffer.resize( m_props->width, m_props->height );
  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  auto proj = m_pCamera->getProjectionMatrix( { m_props->width, m_props->height } );
  m_pRenderingSystem->render( SceneManager::getCurrentScene().lock(), m_pCamera->getViewMatrix(), proj, shader );
  spdlog::info( "pixelData:{}", m_pickingFrameBuffer.readPixel( 0, mx, my ) );
}

void SceneViewportWindow::onKeyPressed( const KeyPressedEvent& e )
{
}

void SceneViewportWindow::draw()
{
  if ( !begin() )
    return;

  m_props->focused = ImGui::IsWindowFocused();
  m_props->hovered = ImGui::IsWindowHovered();

  setupProportions();

  // this must be called every frame, not on key press function because it would never move smoothly
  if ( m_props->focused )
    m_pCamera->onKeyPressed( static_cast<float>( TIME_TRACKER()->getDuration( "deltaTime" ).count() ) );

  ImVec2 contentSize = ImGui::GetContentRegionAvail();

  const auto& scene = SceneManager::getCurrentScene().lock();
  ImVec2 win_pos = ImGui::GetCursorScreenPos();

  const auto& io = ImGui::GetIO();

  drawScene( ImGui::GetWindowSize() );
  drawPickingScene( ImGui::GetWindowSize() );

  ImGui::GetWindowDrawList()->AddImage( (ImTextureID)m_frameBuffer.getColorAttachmentId( 0 ), win_pos,
                                        ImVec2( win_pos.x + contentSize.x, win_pos.y + contentSize.y ), ImVec2( 0, 1 ),
                                        ImVec2( 1, 0 ) );

  // we set the position to top left of this window to prepare for the drop zone
  ImGui::SetCursorScreenPos( win_pos );

  // this is just the viewport drop zone, it is on top of frame buffer so we need to make it invisible
  ImGui::InvisibleButton( "viewportDropZone", contentSize );

  // here we accept drag and drop payload from the assets window
  if ( ImGui::BeginDragDropTarget() )
  {
    // if we have a payload
    const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ASSET_DROP" );
    manageAssetsPayload( payload );
    ImGui::EndDragDropTarget();
  }

  end();
}

void SceneViewportWindow::manageAssetsPayload( const ImGuiPayload* payload ) const
{
  if ( !payload )
  {
    return;
  }

  const auto& pAssetManager = ASSET_MANAGER();
  auto data = static_cast<const char*>( payload->Data );
  std::string dropResult( data, payload->DataSize );
  std::filesystem::path p{ dropResult };

  auto scene = SceneManager::getCurrentScene();
  auto pScene = scene.lock();

  const auto& extension = p.extension().string();
  if ( extension.find( ".gltf" ) != std::string::npos )
  {
    spdlog::info( "dropped a model file from {}, ext:{}", dropResult, p.extension().string() );
    if ( !pScene )
      return;

    // if no entity is selected we create one ad add the ModelComponent to it
    if ( m_selectedEntity == entt::null )
    {
      auto pModel = pAssetManager->addModel( p.filename().string(), p.string() );
      pScene->addEntity( pModel );
      return;
    }
  }
  else if ( extension.find( ".jpg" ) != std::string::npos || extension.find( ".png" ) != std::string::npos )
  {
    spdlog::info( "dropped a texture file from {}, ext:{}", dropResult, p.extension().string() );
    if ( !pScene )
      return;

    // if no entity is selected we don't go further
    if ( m_selectedEntity == entt::null )
      return;

    // we load the texture
    pAssetManager->addTexture( p.filename().string(), p.string() );
  }
  else
  {
    spdlog::info( "format currently unsupported {}", p.extension().string() );
  }
}
} // namespace kogayonon_gui