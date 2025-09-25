#include "gui/scene_viewport.hpp"
#include <codecvt>
#include <filesystem>
#include <glad/glad.h>
#include <spdlog/spdlog.h>
#include "core/ecs/components/texture_component.hpp"
#include "core/ecs/entity.hpp"
#include "core/ecs/main_registry.hpp"
#include "core/event/event_dispatcher.hpp"
#include "core/event/scene_events.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "rendering/framebuffer.hpp"
#include "utilities/asset_manager/asset_manager.hpp"

namespace kogayonon_gui
{
SceneViewportWindow::SceneViewportWindow( std::string name, std::weak_ptr<kogayonon_rendering::FrameBuffer> frameBuffer,
                                          unsigned int playTextureId, unsigned int stopTextureId )
    : ImGuiWindow{ std ::move( name ) }
    , m_pFrameBuffer{ frameBuffer }
    , m_playTextureId{ playTextureId }
    , m_stopTextureId{ stopTextureId }
    , m_selectedEntity{ entt::null }
{
  EVENT_DISPATCHER()->addHandler<kogayonon_core::SelectEntityEvent, &SceneViewportWindow::onSelectedEntity>( *this );
}

void SceneViewportWindow::onSelectedEntity( kogayonon_core::SelectEntityEvent& e )
{
  m_selectedEntity = e.getEntity();
}

std::weak_ptr<kogayonon_rendering::FrameBuffer> SceneViewportWindow::getFrameBuffer()
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

  static constexpr float toolbarHeight = 25.0f;
  ImGui::BeginChild( "Toolbar", ImVec2( 0, toolbarHeight ), false );
  ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) ); // no bg color for the button
  if ( ImGui::ImageButton( "play", m_playTextureId, ImVec2( 18, 18 ) ) )
  {
    spdlog::info( "Pressed play" );
  }
  ImGui::PopStyleColor( 1 );

  ImGui::SameLine();

  ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );
  if ( ImGui::ImageButton( "stop", m_stopTextureId, ImVec2( 18, 18 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ),
                           ImVec4( 0, 0, 0, 0 ), ImVec4( 1, 1, 1, 1 ) ) )
  {
    spdlog::info( "Pressed stop" );
  }
  ImGui::PopStyleColor( 1 );

  ImGui::SameLine();

  // render the scene name
  if ( auto pScene = kogayonon_core::SceneManager::getCurrentScene(); auto scene = pScene.lock() )
  {
    ImGui::Text( "%s", scene->getName().c_str() );
  }

  ImGui::EndChild();

  ImVec2 contentSize = ImGui::GetContentRegionAvail();
  auto pFrameBuffer = m_pFrameBuffer.lock();
  if ( pFrameBuffer && m_renderCallback )
  {
    pFrameBuffer->bind();
    pFrameBuffer->rescale( contentSize.x, contentSize.y );

    m_renderCallback();

    pFrameBuffer->unbind();

    ImVec2 win_pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddImage( (void*)pFrameBuffer->getTexture(), win_pos,
                                          ImVec2( win_pos.x + contentSize.x, win_pos.y + contentSize.y ),
                                          ImVec2( 0, 1 ), ImVec2( 1, 0 ) );

    // we set the position to top left of this window to prepare for the drop zone
    ImGui::SetCursorScreenPos( win_pos );

    // this is just the viewport drop zone, it is after the framebuffer texture so it must remain invisible
    ImGui::InvisibleButton( "viewportDropZone", contentSize );

    // here we accept drag and drop payload from the assets window
    if ( ImGui::BeginDragDropTarget() )
    {
      // if we have a payload
      manageAssetsPayload( ImGui::AcceptDragDropPayload( "ASSET_DROP" ) );
      ImGui::EndDragDropTarget();
    }
  }

  ImGui::End();
}

void SceneViewportWindow::manageAssetsPayload( const ImGuiPayload* payload ) const
{
  if ( payload )
  {
    const auto& pAssetManager = ASSET_MANAGER();
    auto data = static_cast<const char*>( payload->Data );
    std::string dropResult( data, payload->DataSize );
    std::filesystem::path p{ dropResult };
    if ( p.extension().string() == ".gltf" )
    {
      spdlog::info( "dropped a model file from {}, ext:{}", dropResult, p.extension().string() );
      pAssetManager->addModel( p.filename().string(), p.string() );
    }
    else if ( p.extension().string() == ".png" || p.extension().string() == ".jpg" )
    {
      spdlog::info( "dropped a texture file from {}, ext:{}", dropResult, p.extension().string() );
      auto s = kogayonon_core::SceneManager::getCurrentScene();
      if ( auto pScene = s.lock() )
      {
        auto texture = pAssetManager->addTexture( p.filename().string(), p.string() );
        if ( m_selectedEntity != entt::null )
        {
          auto ent = std::make_shared<kogayonon_core::Entity>( pScene->getRegistry(), m_selectedEntity );
          if ( auto textureComponent = ent->tryGetComponent<kogayonon_core::TextureComponent>();
               ent->hasComponent<kogayonon_core::TextureComponent>() )
          {
            std::string texturePath = textureComponent->pTexture.lock()->getPath();
            ent->removeComponent<kogayonon_core::TextureComponent>();
            ASSET_MANAGER()->removeTexture( texturePath );
          }
          ent->addComponent<kogayonon_core::TextureComponent>( texture );
        }
        else
        {
          auto ent = std::make_shared<kogayonon_core::Entity>( pScene->getRegistry(), "Object" );
          ent->addComponent<kogayonon_core::TextureComponent>( texture );
        }
      }
    }
    else
    {
      spdlog::info( "format currently unsupported {}", p.extension().string() );
    }
  }
}
} // namespace kogayonon_gui