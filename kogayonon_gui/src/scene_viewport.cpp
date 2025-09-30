#include "gui/scene_viewport.hpp"
#include <codecvt>
#include <filesystem>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <spdlog/spdlog.h>
#include "core/ecs/components/model_component.hpp"
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/components/transform_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/input/key_codes.hpp"
#include "core/input/keyboard_events.hpp"
#include "core/input/mouse_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "core/systems/rendering_system.h"
#include "imgui_utils/imgui_utils.h"
#include "rendering/camera/camera.hpp"
#include "rendering/framebuffer.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
#include "utilities/shader_manager/shader.hpp"
#include "utilities/shader_manager/shader_manager.hpp"
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
  if ( !m_props || !m_props->focused )
    return;

  const auto& io = ImGui::GetIO();
  if ( io.MouseDown[ImGuiMouseButton_Middle] )
  {
    SDL_SetRelativeMouseMode( SDL_TRUE );
    auto x = static_cast<float>( e.getXRel() );
    auto y = static_cast<float>( e.getYRel() );
    m_pCamera->onMouseMoved( x, y, true );
  }
  else
  {
    SDL_SetRelativeMouseMode( SDL_FALSE );
  }
}

void SceneViewportWindow::onMouseClicked( const MouseClickedEvent& e )
{
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
  if ( !ImGui::Begin( m_props->name.c_str(), nullptr, m_props->flags ) )
  {
    ImGui::End();
    return;
  }

  m_props->focused = ImGui::IsWindowFocused();
  m_props->hovered = ImGui::IsWindowHovered();

  static constexpr float toolbarHeight = 25.0f;
  ImGui::BeginChild( "Toolbar", ImVec2( 0, toolbarHeight ), false );
  if ( ImGui::ImageButton( "play", m_playTextureId, ImVec2( 18, 18 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ),
                           ImVec4( 0, 0, 0, 0 ), ImVec4( 1, 1, 1, 1 ) ) )
  {
    spdlog::info( "Pressed play" );
  }

  ImGui::SameLine();

  if ( ImGui::ImageButton( "stop", m_stopTextureId, ImVec2( 18, 18 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ),
                           ImVec4( 0, 0, 0, 0 ), ImVec4( 1, 1, 1, 1 ) ) )
  {
    spdlog::info( "Pressed stop" );
  }
  ImGui::SameLine();

  // render the scene name
  auto pScene = SceneManager::getCurrentScene();
  auto scene = pScene.lock();
  if ( scene )
  {
    ImGui::Text( "%s", scene->getName().c_str() );
  }

  ImGui::EndChild();

  ImVec2 contentSize = ImGui::GetContentRegionAvail();
  auto pFrameBuffer = m_pFrameBuffer.lock();
  if ( !pFrameBuffer )
  {
    ImGui::End();
    return;
  }

  pFrameBuffer->bind();
  pFrameBuffer->rescale( contentSize.x, contentSize.y );

  // set clear color first
  glClearColor( 0.3f, 0.3f, 0.3f, 1.0f );

  // then clear
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  auto& shader = SHADER_MANAGER()->getShader( "3d" );
  glm::mat4 proj = glm::perspective( glm::radians( 45.0f ), contentSize.x / contentSize.y, 0.1f, 4000.0f );

  auto& viewMatrix = m_pCamera->getViewMatrix();
  m_pRenderingSystem->render( scene, viewMatrix, proj, shader );
  pFrameBuffer->unbind();

  ImVec2 win_pos = ImGui::GetCursorScreenPos();
  ImGui::GetWindowDrawList()->AddImage( (void*)pFrameBuffer->getTexture(), win_pos,
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

  ImGui::End();
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
    auto texture = pAssetManager->addTexture( p.filename().string(), p.string() );

    // we replace the texture with the new one
    auto ent = std::make_shared<Entity>( pScene->getRegistry(), m_selectedEntity );
    if ( auto textureComponent = ent->tryGetComponent<TextureComponent>() )
    {
      std::string texturePath = textureComponent->pTexture.lock()->getPath();
      ASSET_MANAGER()->removeTexture( texturePath );
    }
    ent->replaceComponent<TextureComponent>( texture );
  }
  else
  {
    spdlog::info( "format currently unsupported {}", p.extension().string() );
  }
}
} // namespace kogayonon_gui