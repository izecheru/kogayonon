#include "core/ui/imgui_interface.h"
#include "core/logger.h"

#include <imgui-1.91.8/imgui.h>
#include <imgui-1.91.8/imgui_impl_glfw.h>
#include <imgui-1.91.8/imgui_impl_opengl3.h>

namespace kogayonon
{
  ImguiInterface::ImguiInterface(GLFWwindow* window) {
    if (initImgui(window)) {
      Logger::logInfo("Imgui initialised");
    }
    else {
      Logger::logError("Imgui could not be initialised");
    }
  }

  bool ImguiInterface::initImgui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = "imgui_config.ini";
    (void)io;
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

  void ImguiInterface::draw() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto& window : m_windows) {
      window.draw();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  bool ImguiInterface::createWindow(std::string window_name, double x_pos, double y_pos, bool docked, bool visible) {
    ImguiWindow window(window_name, x_pos, y_pos, visible, docked);
    m_windows.push_back(window);
    return true;
  }

  std::vector<ImguiWindow>& ImguiInterface::getWindows() {
    return m_windows;
  }
}