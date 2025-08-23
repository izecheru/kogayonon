#include "ui/imgui_manager.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include "context_manager/context_manager.h"
#include "ui/scene_viewport.h"
#include "ui/win_camera_settings.h"

namespace kogayonon
{
  ImGuiManager::~ImGuiManager()
  {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    m_windows.clear();
  }

  ImGuiManager::ImGuiManager(GLFWwindow* window)
  {
    if (initImgui(window))
    {
      ContextManager::klogger()->log(LogType::INFO, "Imgui initialised");
      push_window(std::make_shared<CameraSettingsWindow>("Camera settings"));
      push_window(std::make_shared<SceneViewportWindow>("Scene"));
    }
    else
    {
      ContextManager::klogger()->log(LogType::ERROR, "Imgui could not be initialised");
    }
  }

  bool ImGuiManager::initImgui(GLFWwindow* window)
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    m_io = &ImGui::GetIO();
    m_io->ConfigFlags |= ImGuiConfigFlags_DockingEnable | ImGuiConfigFlags_ViewportsEnable;
    m_io->IniFilename = "imgui_config.ini";

    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true) || !ImGui_ImplOpenGL3_Init("#version 460"))
    {
      ContextManager::klogger()->log(LogType::ERROR, "Error init imgui");
      return false;
    }
    return true;
  }

  void ImGuiManager::setupDockSpace(ImGuiViewport* viewport)
  {
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
      ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
      ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dockspace_flags);
      static bool first_frame = true;
      if (first_frame)
      {
        first_frame = false;

        ImGuiID dockspace_id = ImGui::GetID("MyDockspace");

        // Clear previous layout
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, viewport->Size);

        // Split main dockspace
        ImGuiID dock_main_id = dockspace_id;
        ImGuiID dock_id_right, dock_id_left;

        ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.2f, &dock_id_right, &dock_main_id);
        ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.25f, &dock_id_left, &dock_main_id);

        // Dock windows
        ImGui::DockBuilderDockWindow("Camera settings", dock_id_left);
        ImGui::DockBuilderDockWindow("Scene", dock_main_id);

        ImGui::DockBuilderFinish(dockspace_id);
      }
    }
  }

  void ImGuiManager::beginImGuiFrame()
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    static ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                                           ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                           ImGuiWindowFlags_NoNavFocus;

    static ImGuiViewport* viewport = ImGui::GetMainViewport();
    static float menu_bar_height = ImGui::GetFrameHeight();

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menu_bar_height));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, viewport->Size.y - menu_bar_height));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Main window", nullptr, window_flags);
    ImGui::PopStyleVar(2);

    setupDockSpace(viewport);
    ImGui::End();
  }

  void ImGuiManager::endImGuiFrame()
  {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if (m_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
      GLFWwindow* backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }
  }

  void ImGuiManager::draw()
  {
    beginImGuiFrame();
    mainMenu();
    for (auto& win : m_windows)
    {
      win->draw();
    }
    endImGuiFrame();
  }

  void ImGuiManager::mainMenu()
  {
    if (ImGui::BeginMainMenuBar())
    {
      if (ImGui::BeginMenu("File"))
      {
        if (ImGui::MenuItem("Close", "Ctrl+X"))
        {
          ContextManager::klogger()->log(LogType::INFO, "Ctrl+X pressed");
        }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }
  }

  void ImGuiManager::push_window(std::shared_ptr<ImGuiWindow> window)
  {
    m_windows.push_back(window);
  }

  std::vector<std::shared_ptr<ImGuiWindow>>& ImGuiManager::getWindows()
  {
    return m_windows;
  }
} // namespace kogayonon