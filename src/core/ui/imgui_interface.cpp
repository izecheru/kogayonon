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
    for (int i = 0; i < m_windows.size(); i++)
    {
      delete m_windows[i];
    }
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
    ImGuiIO& io    = ImGui::GetIO();

    io.ConfigFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    io.IniFilename = "imgui_config.ini";

    ImGui::StyleColorsLight();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
      ContextManager::klogger()->log(LogType::ERROR, "error init imgui");
      return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 460"))
    {
      ContextManager::klogger()->log(LogType::ERROR, "error init imgui");
      return false;
    }
    return true;
  }

  void ImGuiInterface::draw()
  {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto& window : m_windows)
    {
      window->draw();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  /// <summary>
  /// Initializes all the ImGui windows prepared for the engine and pushes them to
  /// the m_windows vector
  /// </summary>
  /// <returns></returns>

  bool ImGuiInterface::initWindows()
  {
    m_windows.push_back(new CameraSettingsWindow("Camera settings"));
    return true;
  }

  Windows& ImGuiInterface::getWindows()
  {
    return m_windows;
  }
} // namespace kogayonon