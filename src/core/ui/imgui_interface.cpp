#include "core/ui/imgui_interface.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "core/context_manager/context_manager.h"
#include "core/ui/win_camera_settings.h"

namespace kogayonon
{
  ImGuiInterface::~ImGuiInterface()
  {
    m_windows.clear();
  }

  ImGuiInterface::ImGuiInterface(GLFWwindow* window)
  {
    if (initImgui(window))
    {
      ContextManager::klogger()->log(LogType::INFO, "Imgui initialised");
    }
    else
    {
      ContextManager::klogger()->log(LogType::ERROR, "Imgui could not be initialised");
    }
  }

  bool ImGuiInterface::initImgui(GLFWwindow* window)
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Correct: only config flags here
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

    // Window flags go into DockSpace/Begin
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    io.IniFilename = "imgui_config.ini";
    ImGui::StyleColorsLight();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true) || !ImGui_ImplOpenGL3_Init("#version 460"))
    {
      ContextManager::klogger()->log(LogType::ERROR, "error init imgui");
      return false;
    }
    return true;
  }

  void ImGuiInterface::draw()
  {
    // Start new ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Fullscreen DockSpace window
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
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

  /// <summary>
  /// Initializes all the ImGui windows prepared for the engine and pushes them to
  /// the m_windows vector
  /// </summary>
  /// <returns></returns>

  bool ImGuiInterface::initWindows()
  {
    m_windows.push_back(std::make_shared<CameraSettingsWindow>("Camera settings"));
    return true;
  }

  std::vector<std::shared_ptr<ImguiWindow>>& ImGuiInterface::getWindows()
  {
    return m_windows;
  }
} // namespace kogayonon