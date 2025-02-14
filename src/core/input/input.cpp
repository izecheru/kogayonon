#include "app/app.h"
#include "core/input/input.h"

using namespace kogayonon;

bool Input::isKeyPressed(KeyCode key) {
  auto* window = static_cast<GLFWwindow*>(App::getWindow());
  auto state = glfwGetKey(window, static_cast<int32_t>(key));
  return state == GLFW_PRESS;
}

bool Input::isMouseButtonPressed(MouseCode button) {
  auto* window = static_cast<GLFWwindow*>(App::getWindow());
  auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
  return state == GLFW_PRESS;
}

glm::vec2 Input::getMousePos() {
  auto* window = static_cast<GLFWwindow*>(App::getWindow());
  double x_pos, y_pos;
  glfwGetCursorPos(window, &x_pos, &y_pos);
  return { x_pos, y_pos };
}

float Input::getMouseX() {
  return getMousePos().x;
}

float Input::getMouseY() {
  return getMousePos().y;
}

