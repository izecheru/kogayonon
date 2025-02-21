#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "core/ui/win_camera_settings.h"
#include "core/ui/imgui_interface.h"
#include "core/logger.h"

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
      Logger::logInfo("Imgui initialised");
    }
    else
    {
      Logger::logError("Imgui could not be initialised");
    }
  }

  bool ImGuiInterface::initImgui(GLFWwindow* window)
  {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    io.IniFilename = "imgui_config.ini";

    ImGui::StyleColorsDark();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
    {
      Logger::logError("error init imgui");
      return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 460"))
    {
      Logger::logError("error init imgui");
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
}