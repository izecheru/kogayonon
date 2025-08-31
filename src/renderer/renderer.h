#pragma once
#include "framebuffer.h"
#include "shader/shader_manager.h"
#include "ui/debug_window.h"
#include "ui/imgui_manager.h"
#include "ui/scene_viewport.h"
#include "ui/win_camera_settings.h"
#include "window/window.h"

namespace kogayonon
{
/**
 * @brief Renderer class, supports ImGui windows and also has a SceneViewport to which we render
 * the scene
 */
class Renderer
{
public:
  explicit Renderer(std::shared_ptr<Window> window)
      : m_window(window), is_poly(true), m_shader_manager(std::make_unique<ShaderManager>()),
        m_imgui_manager(std::make_unique<ImGuiManager>(window->getWindow(), window->getContext())),
        m_scene_fbo(std::make_shared<FrameBuffer>(800, 600))
  {
    m_imgui_manager->push_window("Scene", std::make_unique<SceneViewportWindow>("Scene", m_scene_fbo));
    m_imgui_manager->push_window("Debug console", std::make_unique<DebugConsoleWindow>("Debug console"));
  }

  ~Renderer() = default;

  void draw();
  bool getPolyMode();
  void togglePolyMode();

  /**
   * @brief Just a test function for the callback
   */
  void callback_test()
  {
    // temp code
    m_shader_manager->bindShader("3d");
    float triangleVertices[] = {0.0f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f};
    static GLuint VAO = 0, VBO = 0; // static so we only create once
    if (VAO == 0)
    {
      glCreateVertexArrays(1, &VAO);
      glCreateBuffers(1, &VBO);
      glNamedBufferStorage(VBO, sizeof(triangleVertices), triangleVertices, 0);
      glVertexArrayVertexBuffer(VAO, 0, VBO, 0, 3 * sizeof(float));

      glEnableVertexArrayAttrib(VAO, 0);
      glVertexArrayAttribFormat(VAO, 0, 3, GL_FLOAT, GL_FALSE, 0);
      glVertexArrayAttribBinding(VAO, 0, 0);
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    m_shader_manager->unbindShader("3d");

    // ---------
  }

  /**
   * @brief Sets the callback for the SceneViewportWindow::draw() and we call it to render whatever
   * func does in the ImGui loop there after frame buffer binding
   * @param func The function that is to be called in the SceneViewportWindow::draw()
   */
  inline void setRenderCallback(const std::function<void()>& func)
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
  std::weak_ptr<Window> m_window;
  std::shared_ptr<FrameBuffer> m_scene_fbo;
  std::unique_ptr<ImGuiManager> m_imgui_manager = nullptr;
  std::unique_ptr<ShaderManager> m_shader_manager = nullptr;
};
} // namespace kogayonon
