#include "gui/scene_viewport.hpp"
#include <codecvt>
#include <glad/glad.h>
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "logger/logger.hpp"
#include "rendering/framebuffer.hpp"

using namespace kogayonon_logger;

namespace kogayonon_gui
{
SceneViewportWindow::SceneViewportWindow( std::string name,
                                          std::shared_ptr<kogayonon_rendering::FrameBuffer> frameBuffer,
                                          unsigned int playTextureId, unsigned int stopTextureId )
    : ImGuiWindow( std ::move( name ) )
    , m_pFrameBuffer( frameBuffer )
    , m_playTextureId( playTextureId )
    , m_stopTextureId( stopTextureId )
{
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
    Logger::info( "Pressed play" );
  }
  ImGui::PopStyleColor( 1 );

  ImGui::SameLine();

  ImGui::PushStyleColor( ImGuiCol_Button, ImVec4( 0, 0, 0, 0 ) );
  if ( ImGui::ImageButton( "stop", m_stopTextureId, ImVec2( 18, 18 ), ImVec2( 0, 0 ), ImVec2( 1, 1 ),
                           ImVec4( 0, 0, 0, 0 ), ImVec4( 1, 1, 1, 1 ) ) )
  {
    Logger::info( "Pressed stop" );
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
      if ( const ImGuiPayload* payload = ImGui::AcceptDragDropPayload( "ASSET_DROP" ) )
      {
        const char* data = static_cast<const char*>( payload->Data );
        std::string dropResult( data, payload->DataSize );
        Logger::info( "dropped payload data ", dropResult );
      }
      ImGui::EndDragDropTarget();
    }
  }

  ImGui::End();
}
} // namespace kogayonon_gui