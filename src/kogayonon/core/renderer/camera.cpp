#include "core/renderer/camera.h"
#include <glm/detail/func_trigonometric.inl>
#include <glm/ext/matrix_transform.inl>
#include <GLFW/glfw3.h>
#include "core/logger.h"
#include "core/input/input.h"

Camera::Camera() {
  m_props.position = glm::vec3(0.0f, 0.0f, 3.0f);
  m_props.direction = glm::vec3(0.0f, 0.0f, -1.0f);
  m_props.camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
  m_props.world_up = glm::vec3(0.0f, 1.0f, 0.0f);
  m_props.yaw = -90.0f;
  m_props.pitch = 0.0f;
  m_props.mouse_sens = 0.5f;
  m_props.movement_speed = 0.1f;
}

void Camera::updateView() {}

void Camera::setView() {}

const CameraProps& Camera::getCamera() {
  return m_props;
}

const bool Camera::getFirstMove() {
  return first_move;
}

void Camera::setFirstMove() {
  first_move = false;
}

const glm::mat4& Camera::getViewMatrix() const {
  static glm::mat4 view;
  view = glm::lookAt(m_props.position, m_props.position + m_props.direction, m_props.camera_up);
  return view;
}

void Camera::processMouseMoved(float x, float y, bool constrain_pitch) {
  static float lastX = x;
  static float lastY = y;

  if (first_move)
  {
    lastX = x;
    lastY = y;
    first_move = false;
  }

  float xoffset = x - lastX;
  float yoffset = lastY - y;
  lastX = x;
  lastY = y;

  xoffset *= m_props.mouse_sens;
  yoffset *= m_props.mouse_sens;

  m_props.yaw += xoffset;
  m_props.pitch += yoffset;

  // this is to no go deaberbeleacu 
  if (constrain_pitch)
  {
    if (m_props.pitch > 89.0f) m_props.pitch = 89.0f;
    if (m_props.pitch < -89.0f) m_props.pitch = -89.0f;
  }

  // Wrap yaw within [-180, 180]
  if (m_props.yaw > 170.0f) m_props.yaw -= 360.0f;
  if (m_props.yaw < -170.0f) m_props.yaw += 360.0f;

  updateCameraVectors();
}

void Camera::processKeyboard(GLFWwindow* window, float delta_time) {
  float velocity = m_props.movement_speed * delta_time * 20.0f;
  if (Input::isKeyPressed(KeyCode::W))
  {
    m_props.position += m_props.direction * velocity;
  }
  if (Input::isKeyPressed(KeyCode::S))
  {
    m_props.position -= m_props.direction * velocity;
  }
  if (Input::isKeyPressed(KeyCode::D))
  {
    m_props.position += m_props.right * velocity;
  }
  if (Input::isKeyPressed(KeyCode::A))
  {
    m_props.position -= m_props.right * velocity;
  }
  if (Input::isKeyPressed(KeyCode::Space))
  {
    m_props.position.y += 1.4f * velocity;
  }
  if (Input::isKeyPressed(KeyCode::LeftControl))
  {
    m_props.position.y -= 1.4f * velocity;
  }
  //m_props.position.y = 0.0f;
  updateCameraVectors();
}

void Camera::updateCameraVectors() {
  // Calculate the new Front vector
  glm::vec3 direction;
  direction.x = cos(glm::radians(m_props.yaw)) * cos(glm::radians(m_props.pitch));
  direction.y = sin(glm::radians(m_props.pitch));
  direction.z = sin(glm::radians(m_props.yaw)) * cos(glm::radians(m_props.pitch));
  m_props.direction = glm::normalize(direction);

  // Recalculate Right and Up vectors
  m_props.right = glm::normalize(glm::cross(m_props.direction, m_props.world_up));
  m_props.camera_up = glm::normalize(glm::cross(m_props.right, m_props.direction));
}

