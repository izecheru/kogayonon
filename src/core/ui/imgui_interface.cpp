#include "core/ui/imgui_interface.h"
#include "core/logger.h"

#include <imgui-1.91.8/imgui.h>
#include <imgui-1.91.8/imgui_impl_glfw.h>
#include <imgui-1.91.8/imgui_impl_opengl3.h>

namespace kogayonon
{
  MyImguiInterface::MyImguiInterface(GLFWwindow* window) {
    if (initImgui(window))
    {
      Logger::logInfo("Imgui initialised");
    }
    else
    {
      Logger::logError("Imgui could not be initialised");
    }
  }

  bool MyImguiInterface::initImgui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true))return false;
    if (!ImGui_ImplOpenGL3_Init("#version 460"))return false;
    return true;
  }

  void MyImguiInterface::draw() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    for (auto& window : m_imgui_windows)
    {
      window.draw();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  bool kogayonon::MyImguiInterface::createWindow(std::string window_name, double x_pos, double y_pos, bool docked, bool visible) {
    ImguiWindow window(window_name, x_pos, y_pos, visible, docked);
    m_imgui_windows.push_back(window);
    return true;
  }
}
