
#include "core/layer/imgui_layer.h"
#include "core/logger.h"
#include "events/keyboard_events.h"

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
  if (event.getEventType() == EventType::KeyPressed)
  {
    KeyPressedEvent& key_event = static_cast<KeyPressedEvent&>(event);
    if (key_event.getKeyCode() == KeyCode::Escape)
    {
      Logger::logInfo("escaped from imgui");
      return true; // we handled the event here
    }
  }
  Logger::logInfo("event received in imgui layer but not processed here");
  return false;
}

void ImguiLayer::onRender() {
  m_interface.draw();
}

void ImguiLayer::onUpdate() {
  Logger::logInfo("on update...");
}