#include "core/renderer/camera.h"
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <glad/glad.h>
#include "core/logger.h"
#include "core/input/input.h"
#include "shader/shader.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace kogayonon
{


  Camera::Camera() {
    m_props = {};
    setView();
  }

  void Camera::setView() {
    m_props.position = glm::vec3(0.0f, 0.0f, 3.0f);
    m_props.direction = glm::vec3(0.0f, 0.0f, -1.0f);
    m_props.camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_props.world_up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_props.yaw = -90.0f;
    m_props.pitch = 0.0f;
    m_props.mouse_sens = 0.2f;
    m_props.movement_speed = 0.8f;
  }

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

    // wrap yaw within [-180, 180]
    if (m_props.yaw > 180.0f) m_props.yaw -= 360.0f;
    if (m_props.yaw < -180.0f) m_props.yaw += 360.0f;

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
    // this is for later when i add jump mechanic 
    //m_props.position.y = 0.0f;
    updateCameraVectors();
  }

  void Camera::updateCameraVectors() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(m_props.yaw)) * cos(glm::radians(m_props.pitch));
    direction.y = sin(glm::radians(m_props.pitch));
    direction.z = sin(glm::radians(m_props.yaw)) * cos(glm::radians(m_props.pitch));
    m_props.direction = glm::normalize(direction);

    m_props.right = glm::normalize(glm::cross(m_props.direction, m_props.world_up));
    m_props.camera_up = glm::normalize(glm::cross(m_props.right, m_props.direction));
  }

  void Camera::cameraUniform(unsigned int shader_id, const char* uniform) {
    int view_location = glGetUniformLocation(shader_id, uniform);
    glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(getViewMatrix()));
  }

  glm::vec3 Camera::getCameraPos() const {
    glm::vec3 pos = glm::vec3(m_props.position.x, m_props.position.y, m_props.position.z);
    return pos;
  }
}
