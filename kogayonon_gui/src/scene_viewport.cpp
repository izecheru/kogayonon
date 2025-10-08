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
  FramebufferSpecification spec{ { FramebufferTextureFormat::RGBA8 }, 800, 800 };
  FramebufferSpecification pickingSpec{
    { FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::DEPTH }, 800, 800 };
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

void SceneViewportWindow::onMouseClicked( const MouseClickedEvent& e )
{
  if ( e.getButton() != MouseCode::BUTTON_1 )
    return;

  drawPickingScene();
}

void SceneViewportWindow::drawScene()
{
  auto& spec = m_frameBuffer.getSpecification();
  m_frameBuffer.bind();

  m_frameBuffer.resize( static_cast<int>( m_props->width ), static_cast<int>( m_props->height ) );
  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  auto& shader = SHADER_MANAGER()->getShader( "3d" );
  const auto& size = ImGui::GetWindowSize();
  glm::mat4 proj = m_pCamera->getProjectionMatrix( { size.x, size.y } );
  m_pRenderingSystem->render( SceneManager::getCurrentScene().lock(), m_pCamera->getViewMatrix(), proj, shader );
  m_frameBuffer.unbind();
}

void SceneViewportWindow::drawPickingScene()
{
  const auto& io = ImGui::GetIO();
  auto [mx, my] = ImGui::GetMousePos();

  mx -= static_cast<float>( m_props->x );
  my -= static_cast<float>( m_props->y );

  float width = m_props->width;
  float height = m_props->height;

  if ( mx < 0 || my < 0 || mx > width || my > height )
    return;

  auto& shader = SHADER_MANAGER()->getShader( "picking" );
  m_pickingFrameBuffer.resize( m_props->width, m_props->height );

  auto proj = m_pCamera->getProjectionMatrix( { m_props->width, m_props->height } );
  m_pickingFrameBuffer.bind();
  glClear( GL_DEPTH_BUFFER_BIT );
  m_pickingFrameBuffer.clearColorAttachment( 0, -1 );
  m_pRenderingSystem->render( SceneManager::getCurrentScene().lock(), m_pCamera->getViewMatrix(), proj, shader );
  int result = m_pickingFrameBuffer.readPixel( 0, mx, my );
  m_pickingFrameBuffer.unbind();

  if ( const auto& scene = SceneManager::getCurrentScene().lock() )
  {
    auto ent = static_cast<entt::entity>( result );
    if ( scene->getRegistry().isValid( ent ) )
    {
      // select
      EVENT_DISPATCHER()->emitEvent( SelectEntityInViewportEvent{ ent } );
    }
    else
    {
      // deselect
      EVENT_DISPATCHER()->emitEvent( SelectEntityInViewportEvent{} );
    }
  }
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
  // also the keyboard button check is a map of keys with a bool set to true if button is currently pressed
  if ( m_props->focused )
    m_pCamera->onKeyPressed( static_cast<float>( TIME_TRACKER()->getDuration( "deltaTime" ).count() ) );

  ImVec2 contentSize = ImGui::GetContentRegionAvail();

  const auto& scene = SceneManager::getCurrentScene().lock();
  ImVec2 win_pos = ImGui::GetCursorScreenPos();

  drawScene();

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