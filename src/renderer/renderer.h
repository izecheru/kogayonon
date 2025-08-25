#pragma once
#include "framebuffer.h"
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
        m_imgui_manager(std::make_unique<ImGuiManager>(window->getWindow(), window->getContext(), std::make_shared<FrameBuffer>()))
  {}

  ~Renderer() = default;

  void draw();
  bool getPolyMode();
  void togglePolyMode();

  void callback_test()
  {
    // temp code
    m_shader_manager->bindShader("3d");
    float triangleVertices[] = {0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f};
    static GLuint VAO = 0, VBO = 0; // static so we only create once
    if (VAO == 0)
    {
      glGenVertexArrays(1, &VAO);
      glGenBuffers(1, &VBO);

      glBindVertexArray(VAO);
      glBindBuffer(GL_ARRAY_BUFFER, VBO);
      glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

      glEnableVertexAttribArray(0); // location = 0 in vertex shader
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindVertexArray(0);
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    m_shader_manager->unbindShader("3d");

    // ---------
  }

  void setRenderCallback(const std::function<void()>& func)
  {
    static bool first_time = true;
    if (first_time)
    {
      auto& windows = m_imgui_manager->getWindows();

      // imguiwindow has a function<void()> callback to pass a rendering function to the scene viewport
      // in the middle of the function we call the callback to prepare the texture for rendering
      if (auto& it = windows.find("Scene"); it != windows.end())
      {
        it->second->setCallback(func);
      }
    }
  }

private:
  bool is_poly = false;
  std::shared_ptr<Window> m_window;
  std::unique_ptr<ImGuiManager> m_imgui_manager = nullptr;
  std::unique_ptr<ShaderManager> m_shader_manager = nullptr;
};
} // namespace kogayonon
