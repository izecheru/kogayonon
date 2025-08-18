#pragma once
#include "core/ui/imgui_manager.h"
#include "shader/shader_manager.h"

namespace kogayonon
{
  class Renderer
  {
  public:
    explicit Renderer(GLFWwindow* window)
        : is_poly(false), m_shader_manager(std::make_unique<ShaderManager>()), m_imgui_manager(std::make_unique<ImGuiManager>(window))
    {}

    ~Renderer()
    {
      m_shader_manager.reset();
      m_shader_manager = nullptr;
    }

    void draw() const;
    bool getPolyMode();
    void togglePolyMode();

  private:
    bool is_poly = false;
    std::unique_ptr<ShaderManager> m_shader_manager = nullptr;
    std::unique_ptr<ImGuiManager> m_imgui_manager = nullptr;
  };
} // namespace kogayonon
