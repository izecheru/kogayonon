#include "gui/scene_viewport.hpp"
#include <glad/glad.h>
#include "core/ecs/main_registry.hpp"
#include "core/scene/scene.hpp"
#include "core/scene/scene_manager.hpp"
#include "logger/logger.hpp"
#include "rendering/framebuffer.hpp"
#include "utilities/asset_manager/asset_manager.hpp"
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

  static constexpr float toolbarHeight = 65.0f;
  static ImTextureRef play;
  play._TexID = m_playTextureId;
  static ImTextureRef stop;
  stop._TexID = m_stopTextureId;
  ImGui::BeginChild( "Toolbar", ImVec2( 0, toolbarHeight ), false );
  {
    if ( ImGui::ImageButton( "play", play, ImVec2( 18, 18 ) ) )
    {
      Logger::info( "Pressed play" );
    }
    ImGui::SameLine();
    if ( ImGui::ImageButton( "stop", stop, ImVec2( 18, 18 ) ) )
    {
      Logger::info( "Pressed stop" );
    }
    ImGui::SameLine();

    if ( auto& pScene = kogayonon_core::SceneManager::getInstance().getCurrentScene(); auto scene = pScene.lock() )
    {
      ImGui::Text( "%s", scene->getName().c_str() );
    }
  }
  ImGui::EndChild();

  ImVec2 contentSize = ImGui::GetContentRegionAvail();
  auto pFrameBuffer = m_pFrameBuffer.lock();
  if ( pFrameBuffer && m_renderCallback )
  {
    pFrameBuffer->bind();
    pFrameBuffer->rescale( contentSize.x, contentSize.y );

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    m_renderCallback();

    pFrameBuffer->unbind();

    ImVec2 win_pos = ImGui::GetCursorScreenPos();
    ImGui::GetWindowDrawList()->AddImage( (void*)pFrameBuffer->getTexture(), win_pos,
                                          ImVec2( win_pos.x + contentSize.x, win_pos.y + contentSize.y ),
                                          ImVec2( 0, 1 ), ImVec2( 1, 0 ) );
  }

  ImGui::End();
}
} // namespace kogayonon_gui