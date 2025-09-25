#pragma once
#include <entt/entt.hpp>
#include "imgui_window.hpp"

namespace kogayonon_rendering
{
class FrameBuffer;
} // namespace kogayonon_rendering

namespace kogayonon_core
{
class Scene;
class SelectEntityEvent;
} // namespace kogayonon_core

namespace kogayonon_gui
{
class SceneViewportWindow : public ImGuiWindow
{
public:
  explicit SceneViewportWindow( std::string name, std::weak_ptr<kogayonon_rendering::FrameBuffer> frameBuffer,
                                unsigned int playTexture, unsigned int stopTexture );
  ~SceneViewportWindow() = default;

  void draw() override;
  std::weak_ptr<kogayonon_rendering::FrameBuffer> getFrameBuffer();

  void onSelectedEntity( kogayonon_core::SelectEntityEvent& e );

  /**
   * @brief Processes the payload from the Assets window
   * @param payload The payload we drag and dropped on the viewport
   */
  void manageAssetsPayload( const ImGuiPayload* payload ) const;

private:
  entt::entity m_selectedEntity;
  std::weak_ptr<kogayonon_rendering::FrameBuffer> m_pFrameBuffer;
  unsigned int m_playTextureId = 0, m_stopTextureId = 0;
};
} // namespace kogayonon_gui