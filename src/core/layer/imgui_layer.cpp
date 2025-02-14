
#include "core/layer/imgui_layer.h"
#include "core/logger.h"

using namespace kogayonon;

ImguiLayer::ImguiLayer(GLFWwindow* window) {
  initLayer(window);
}

bool ImguiLayer::initLayer(GLFWwindow* window) {
  if (!m_interface.initImgui(window))
    return false;

  m_interface.createWindow("Test from layer", 10.0f, 20.0f);

  Logger::logInfo("ImGui layer initialised");
  return true;
}

bool ImguiLayer::onEvent(Event& event) {
  Logger::logInfo("event received");
  return true;
}

void ImguiLayer::onRender() {
  m_interface.draw();
}

void ImguiLayer::onUpdate() {
  Logger::logInfo("on update...");
}