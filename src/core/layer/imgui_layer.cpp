#include "core/layer/imgui_layer.h"
#include "core/logger.h"
#include "events/keyboard_events.h"
#include "events/event_listener.h"

using namespace kogayonon;

ImguiLayer::ImguiLayer(GLFWwindow* window) {
  initLayer(window);
  EventListener::getInstance().subscribe<KeyPressedEvent>([this](Event& e) { this->onKeyPressed(static_cast<KeyPressedEvent&>(e)); });
}

bool ImguiLayer::initLayer(GLFWwindow* window) {
  if (!m_interface.initImgui(window))
    return false;

  m_interface.createWindow("Test from layer", 10.0f, 20.0f);

  Logger::logInfo("ImGui layer initialised");
  return true;
}

bool ImguiLayer::onKeyPressed(KeyPressedEvent& event) {
  if (event.getKeyCode() == KeyCode::Escape)
  {
    Logger::logInfo("escaped from imgui ", m_visible);
    switch (m_visible)
    {
      case true:
        setVisible(false);
        break;
      case false:
        setVisible(true);
        break;
    }
    return true; // we handled the event here
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