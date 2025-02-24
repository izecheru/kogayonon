#include "core/layer/imgui_layer.h"
#include "core/logger.h"
#include "events/keyboard_events.h"
#include "events/event_listener.h"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace kogayonon
{
  ImguiLayer::ImguiLayer(GLFWwindow* window)
  {
    initLayer(window);
    EventListener::getInstance().addCallback<KeyPressedEvent>([this](Event& e) { return this->onKeyPressed(static_cast<KeyPressedEvent&>(e)); });
    EventListener::getInstance().addCallback<MouseClickedEvent>([this](Event& e) { return this->onMouseClicked(static_cast<MouseClickedEvent&>(e)); });
    EventListener::getInstance().addCallback<MouseMovedEvent>([this](Event& e) { return this->onMouseMoved(static_cast<MouseMovedEvent&>(e)); });
  }

  bool ImguiLayer::initLayer(GLFWwindow* window)
  {
    // if this is false, abort
    assert(m_interface.initImgui(window) == true);

    m_interface.initWindows();

    Logger::logInfo("ImGui layer initialised");
    return true;
  }

  bool ImguiLayer::onKeyPressed(KeyPressedEvent& event)
  {
    if (event.getKeyCode() == KeyCode::F2)
    {
      m_visible = !m_visible;
      return true;
    }
    return false;
  }

  bool ImguiLayer::onMouseMoved(MouseMovedEvent& event)
  {
    if (!m_visible) return false;

    //TODO get the window the imgui wants to capture mouse on to enable/ disable mouse
    // capture, if i set a bool flag of flalse in a window, game should take input even
    // after hovering that specific window
    if (ImGui::GetIO().WantCaptureMouse)
    {
      return true;
    }

    return false;
  }

  bool ImguiLayer::onMouseClicked(MouseClickedEvent& event)
  {
    return true;
  }

  void ImguiLayer::draw()
  {
    m_interface.draw();
  }
}