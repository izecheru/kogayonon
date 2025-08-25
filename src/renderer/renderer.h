#pragma once
#include "shader/shader_manager.h"
#include "ui/imgui_manager.h"
#include "window/window.h"

namespace kogayonon
{
class Renderer
{
public:
  explicit Renderer(std::shared_ptr<Window> window)
      : m_window(window), is_poly(false), m_shader_manager(std::make_unique<ShaderManager>()),
        m_imgui_manager(std::make_unique<ImGuiManager>(window->getWindow(), window->getContext()))
  {}

  ~Renderer() = default;

  void draw() const;
  bool getPolyMode();
  void togglePolyMode();

private:
  bool is_poly = false;
  std::shared_ptr<Window> m_window;
  std::unique_ptr<ShaderManager> m_shader_manager = nullptr;
  std::unique_ptr<ImGuiManager> m_imgui_manager = nullptr;
};
} // namespace kogayonon
