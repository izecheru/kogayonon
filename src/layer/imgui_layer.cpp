#include "layer/imgui_layer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "context_manager/context_manager.h"
#include "input/keyboard_events.h"
#include "klogger/klogger.h"

namespace kogayonon
{
  void ImguiLayer::push_window(std::shared_ptr<ImGuiWindow>& window)
  {
    m_windows.push_back(window);
  }

  ImguiLayer::ImguiLayer() {}

  void ImguiLayer::draw()
  {
    // Start new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Fullscreen DockSpace window
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    auto viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("DockSpace Window", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    // Create DockSpace
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
      ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
      ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);
    }

    ImGui::End(); // End DockSpace window

    // Draw all custom windows
    for (auto& window : m_windows)
    {
      if (window)
        window->draw();
    }

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Optional: handle multi-viewport windows
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }
  }
} // namespace kogayonon