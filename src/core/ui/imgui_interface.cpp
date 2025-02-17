#include "core/ui/imgui_interface.h"
#include "core/logger.h"

#include <imgui-1.91.8/imgui.h>
#include <imgui-1.91.8/imgui_impl_glfw.h>
#include <imgui-1.91.8/imgui_impl_opengl3.h>

namespace kogayonon
{
  ImGuiInterface::ImGuiInterface(GLFWwindow* window) {
    if (initImgui(window)) {
      Logger::logInfo("Imgui initialised");
    }
    else {
      Logger::logError("Imgui could not be initialised");
    }
  }

  bool ImGuiInterface::initImgui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    io.IniFilename = "imgui_config.ini";

    ImGui::StyleColorsDark();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
      Logger::logError("error init imgui");
      return false;
    }
    if (!ImGui_ImplOpenGL3_Init("#version 460")) {
      Logger::logError("error init imgui");
      return false;
    }
    return true;
  }

  void ImGuiInterface::draw() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto& window : m_windows) {
      window.draw();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  bool ImGuiInterface::createWindow(std::string window_name, double x_pos, double y_pos) {
    m_windows.push_back(ImguiWindow(window_name));
    return true;
  }

  std::vector<ImguiWindow>& ImGuiInterface::getWindows() {
    return m_windows;
  }
}