#include "core/layer/imgui_layer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "core/context_manager/context_manager.h"
#include "core/klogger/klogger.h"
#include "event/event_listener.h"
#include "event/keyboard_events.h"

namespace kogayonon
{
  void ImguiLayer::push_window(std::shared_ptr<ImGuiWindow>& window)
  {
    m_windows.push_back(window);
  }

  ImguiLayer::ImguiLayer()
  {
    EventListener::getInstance()->addCallback<KeyPressedEvent>(
        [this](Event& e) { return this->onKeyPressed(static_cast<KeyPressedEvent&>(e)); });
    EventListener::getInstance()->addCallback<MouseClickedEvent>(
        [this](Event& e) { return this->onMouseClicked(static_cast<MouseClickedEvent&>(e)); });
    EventListener::getInstance()->addCallback<MouseMovedEvent>(
        [this](Event& e) { return this->onMouseMoved(static_cast<MouseMovedEvent&>(e)); });
  }

  bool ImguiLayer::onKeyPressed(KeyPressedEvent& event)
  {
    if (event.getKeyCode() == KeyCode::F2)
    {
      m_visible = !m_visible;
      return true;
    }
    return false;
  }

  bool ImguiLayer::onMouseMoved(MouseMovedEvent& event)
  {
    if (!m_visible)
      return false;

    // TODO get the window the imgui wants to capture mouse on to enable/ disable mouse
    //  capture, if i set a bool flag of flalse in a window, game should take input even
    //  after hovering that specific window
    if (ImGui::GetIO().WantCaptureMouse)
    {
      return true;
    }

    return false;
  }

  bool ImguiLayer::onMouseClicked(MouseClickedEvent& event)
  {
    return true;
  }

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