#include <core/renderer/camera.h>
#include <glm/ext/matrix_transform.hpp>
Camera::Camera() {
  m_props.camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
  m_props.camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
  m_props.camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
  m_props.world_up = glm::vec3(0.0f, 1.0f, 0.0f);
  m_props.yaw = -90.0f; // Default yaw
  m_props.pitch = 0.0f; // Default pitch
  m_props.mouse_sens = 0.1f; // Default mouse sensitivity
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
  static glm::mat4 view; // Avoid returning a reference to a temporary object
  view = glm::lookAt(m_props.camera_pos, m_props.camera_pos + m_props.camera_front, m_props.camera_up);
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
  float yoffset = lastY - y; // Reversed since y-coordinates go from bottom to top
  lastX = x;
  lastY = y;

  xoffset *= m_props.mouse_sens;
  yoffset *= m_props.mouse_sens;

  m_props.yaw += xoffset;
  m_props.pitch += yoffset;

  if (constrain_pitch)
  {
    if (m_props.pitch > 89.0f) m_props.pitch = 89.0f;
    if (m_props.pitch < -89.0f) m_props.pitch = -89.0f;
  }

  updateCameraVectors();
}

void Camera::updateCameraVectors() {
 // Calculate the new Front vector
  glm::vec3 front;
  front.x = cos(glm::radians(m_props.yaw)) * cos(glm::radians(m_props.pitch));
  front.y = sin(glm::radians(m_props.pitch));
  front.z = sin(glm::radians(m_props.yaw)) * cos(glm::radians(m_props.pitch));
  m_props.camera_front = glm::normalize(front);

  // Recalculate Right and Up vectors
  m_props.right = glm::normalize(glm::cross(m_props.camera_front, m_props.world_up));
  m_props.camera_up = glm::normalize(glm::cross(m_props.right, m_props.camera_front));
}
