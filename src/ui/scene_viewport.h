#pragma once
#include <functional>

#include "imgui_window.h"
#include "renderer/framebuffer.h"

namespace kogayonon
{
class SceneViewportWindow : public ImGuiWindow
{
public:
  SceneViewportWindow(std::string&& name) : ImGuiWindow(std ::move(name))
  {
    m_fbo = std::make_shared<FrameBuffer>(800, 600);
  }

  ~SceneViewportWindow() {}

  void draw() override;

private:
  std::shared_ptr<FrameBuffer> m_fbo;
};
} // namespace kogayonon