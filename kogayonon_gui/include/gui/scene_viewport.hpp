#pragma once
#include "imgui_window.hpp"

namespace kogayonon_rendering
{
class FrameBuffer;
}

namespace kogayonon_core
{
class Scene;
}

namespace kogayonon_gui
{
class SceneViewportWindow : public ImGuiWindow
{
public:
  explicit SceneViewportWindow( std::string name, std::shared_ptr<kogayonon_rendering::FrameBuffer> frameBuffer,
                                unsigned int playTexture, unsigned int stopTexture );
  ~SceneViewportWindow() = default;

  void draw() override;
  std::weak_ptr<kogayonon_rendering::FrameBuffer> getFrameBuffer();

private:
  std::weak_ptr<kogayonon_rendering::FrameBuffer> m_pFrameBuffer;
  unsigned int m_playTextureId = 0, m_stopTextureId = 0;
};
} // namespace kogayonon_gui