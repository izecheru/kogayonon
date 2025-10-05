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
#include "rendering/framebuffer.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/shader_manager/shader_manager.hpp"
#include "utilities/task_manager/task_manager.hpp"
#include "utilities/time_tracker/time_tracker.hpp"

using namespace kogayonon_core;

namespace kogayonon_gui
{
SceneViewportWindow::SceneViewportWindow( SDL_Window* mainWindow, std::string name,
                                          std::weak_ptr<kogayonon_rendering::FrameBuffer> frameBuffer,
                                          unsigned int playTextureId, unsigned int stopTextureId )
    : ImGuiWindow{ std ::move( name ) }
    , m_selectedEntity{ entt::null }
    , m_pFrameBuffer{ frameBuffer }
    , m_playTextureId{ playTextureId }
    , m_stopTextureId{ stopTextureId }
    , m_mainWindow{ mainWindow }
    , m_pRenderingSystem{ std::make_unique<RenderingSystem>() }
    , m_pCamera{ std::make_unique<kogayonon_rendering::Camera>() }
{
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
  if ( !m_props->focused )
    return;

  const auto& io = ImGui::GetIO();
  if ( io.MouseDown[ImGuiMouseButton_Middle] )
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

  auto width = static_cast<float>( m_pFrameBuffer.lock()->getWidth() );
  auto height = static_cast<float>( m_pFrameBuffer.lock()->getHeight() );

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
  // trace only on left click
  if ( e.getButton() == MouseCode::BUTTON_1 )
    traceRay();
}

void SceneViewportWindow::onKeyPressed( const KeyPressedEvent& e )
{
}

std::weak_ptr<kogayonon_rendering::FrameBuffer> SceneViewportWindow::getFrameBuffer() const
{
  return std::weak_ptr<kogayonon_rendering::FrameBuffer>( m_pFrameBuffer );
}

void SceneViewportWindow::draw()
{
  begin();
  m_props->focused = ImGui::IsWindowFocused();
  m_props->hovered = ImGui::IsWindowHovered();

  setupProportions();

  // this must be called every frame, not on key press function because it would never move smoothly
  if ( m_props->focused )
    m_pCamera->onKeyPressed( static_cast<float>( TIME_TRACKER()->getDuration( "deltaTime" ).count() ) );

  ImVec2 contentSize = ImGui::GetContentRegionAvail();
  auto pFrameBuffer = m_pFrameBuffer.lock();
  if ( !pFrameBuffer )
  {
    ImGui::End();
    return;
  }

  const auto& scene = SceneManager::getCurrentScene().lock();
  pFrameBuffer->bind();
  pFrameBuffer->rescale( static_cast<int>( contentSize.x ), static_cast<int>( contentSize.y ) );

  // set clear color first
  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );

  // then clear
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  auto& shader = SHADER_MANAGER()->getShader( "3d" );
  const auto& size = glm::vec2{
    m_pFrameBuffer.lock()->getWidth(),
    m_pFrameBuffer.lock()->getHeight(),
  };
  glm::mat4 proj = m_pCamera->getProjectionMatrix( size );
  m_pRenderingSystem->render( scene, m_pCamera->getViewMatrix(), proj, shader );
  pFrameBuffer->unbind();

  ImVec2 win_pos = ImGui::GetCursorScreenPos();
  ImGui::GetWindowDrawList()->AddImage( (ImTextureID)pFrameBuffer->getTexture(), win_pos,
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
    manageAssetsPayload( ImGui::AcceptDragDropPayload( "ASSET_DROP" ) );
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

  if ( p.extension().string() == ".gltf" )
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
  else if ( p.extension().string() == ".png" || p.extension().string() == ".jpg" )
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