#include "core/layer/imgui_layer.h"
#include "core/logger.h"
#include "events/keyboard_events.h"
#include "events/event_listener.h"
#include <imgui-1.91.8/imgui.h>
#include <imgui-1.91.8/imgui_impl_glfw.h>
#include <imgui-1.91.8/imgui_impl_opengl3.h>

namespace kogayonon
{
  ImguiLayer::ImguiLayer(GLFWwindow* window) {
    initLayer(window);
    EventListener::getInstance().addCallback<KeyPressedEvent>([this](Event& e) { return this->onKeyPressed(static_cast<KeyPressedEvent&>(e)); });
    EventListener::getInstance().addCallback<MouseClickedEvent>([this](Event& e) { return this->onMouseClicked(static_cast<MouseClickedEvent&>(e)); });
    EventListener::getInstance().addCallback<MouseMovedEvent>([this](Event& e) { return this->onMouseMoved(static_cast<MouseMovedEvent&>(e)); });
  }

  bool ImguiLayer::initLayer(GLFWwindow* window) {
    if (!m_interface.initImgui(window))
      return false;

    m_interface.createWindow("Test from layer", 10.0f, 20.0f);

    Logger::logInfo("ImGui layer initialised");
    return true;
  }

  bool ImguiLayer::onKeyPressed(KeyPressedEvent& event) {
    if (event.getKeyCode() == KeyCode::F2) {
      m_visible = !m_visible;
      return true;
    }
    return false;
  }

  bool ImguiLayer::onMouseMoved(MouseMovedEvent& event) {
    if (!m_visible) return false;
    if (event.isHandled() == true) return false;

    if (ImGui::GetIO().WantCaptureMouse) {
      return true;
    }

    return false;
  }

  bool ImguiLayer::onMouseClicked(MouseClickedEvent& event) {
    return true;
  }

  void ImguiLayer::render() {
    m_interface.draw();
  }

  void ImguiLayer::onUpdate() {
    Logger::logInfo("on update...");
  }
}