#pragma once
#include <unordered_map>

#include "imgui_window.h"
#include "klogger/klogger.h"
#include "renderer/framebuffer.h"

namespace kogayonon
{
class SceneViewportWindow : public ImGuiWindow
{
public:
  SceneViewportWindow(std::string&& name, std::shared_ptr<FrameBuffer> fbo) : ImGuiWindow(std ::move(name)), m_fbo(fbo) {}

  ~SceneViewportWindow() {}

  void draw() override;

private:
  std::weak_ptr<FrameBuffer> m_fbo;
};
} // namespace kogayonon