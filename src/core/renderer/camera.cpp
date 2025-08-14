#include "core/renderer/camera.h"
#ifndef GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_NONE
#endif
#include <glad/glad.h>
#include <imgui.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/input/input.h"
#include "core/klogger/klogger.h"
#include "core/time_tracker/time_tracker.h"
#include "event/event_listener.h"
#include "event/keyboard_events.h"
#include "shader/shader.h"

namespace kogayonon
{
  Camera::Camera()
  {
    EventListener::getInstance()->addCallback<MouseScrolledEvent>(
        [this](Event& e) { return this->onMouseScrolled(static_cast<MouseScrolledEvent&>(e)); });

    // EventListener::getInstance().addCallback<MouseClickedEvent>([this](Event& e)
    //{
    // return this->onMouseClicked(static_cast<MouseClickedEvent&>(e));
    //});
    setupCamera();
  }

  void Camera::setupCamera()
  {
    m_props.position       = glm::vec3(0.0f, 0.0f, 3.0f);
    m_props.direction      = glm::vec3(0.0f, 0.0f, -1.0f);
    m_props.camera_up      = glm::vec3(0.0f, 1.0f, 0.0f);
    m_props.world_up       = glm::vec3(0.0f, 1.0f, 0.0f);
    m_props.yaw            = -90.0f;
    m_props.pitch          = 0.0f;
    m_props.mouse_sens     = 0.2f;
    m_props.movement_speed = 90.0f;
  }

  const CameraProps& Camera::getCamera()
  {
    return m_props;
  }

  const bool Camera::getFirstMove()
  {
    return first_move;
  }

  void Camera::setFirstMove()
  {
    first_move = false;
  }

  const glm::mat4& Camera::getViewMatrix() const
  {
    static glm::mat4 view;
    view = glm::lookAt(m_props.position, m_props.position + m_props.direction, m_props.camera_up);
    return view;
  }

  bool Camera::onMouseScrolled(MouseScrolledEvent& event)
  {
    if (event.isHandled())
      return false;
    processMouseScrolled(event.getXOff(), event.getYOff());
    return true;
  }

  void Camera::processMouseScrolled(double x_offset, double y_offset)
  {
    float velocity = m_props.movement_speed * TimeTracker::getInstance()->getDelta() * 100.0f;
    m_props.position.z -= y_offset * velocity;
    updateCameraVectors();
  }

  bool Camera::onMouseMoved(MouseMovedEvent& event)
  {
    if (event.isHandled())
      return false;
    processMouseMoved(event.getX(), event.getY());
    return true;
  }

  void Camera::processMouseMoved(float x, float y, bool constrain_pitch)
  {
    static float lastX = x;
    static float lastY = y;

    if (first_move)
    {
      lastX      = x;
      lastY      = y;
      first_move = false;
    }

    float xoffset = x - lastX;
    float yoffset = lastY - y;
    lastX         = x;
    lastY         = y;

    xoffset *= m_props.mouse_sens;
    yoffset *= m_props.mouse_sens;

    m_props.yaw += xoffset;
    m_props.pitch += yoffset;

    // this is to no go deaberbeleacu
    if (constrain_pitch)
    {
      if (m_props.pitch > 89.0f)
        m_props.pitch = 89.0f;
      if (m_props.pitch < -89.0f)
        m_props.pitch = -89.0f;
    }

    // wrap yaw within [-180, 180]
    if (m_props.yaw > 180.0f)
      m_props.yaw -= 360.0f;
    if (m_props.yaw < -180.0f)
      m_props.yaw += 360.0f;

    updateCameraVectors();
  }

  void Camera::processKeyboard()
  {
    float velocity = m_props.movement_speed * TimeTracker::getInstance()->getDelta() * 30.0f;

    if (KeyboardState::getKeyState(KeyCode::W))
    {
      m_props.position += m_props.direction * velocity;
    }
    if (KeyboardState::getKeyState(KeyCode::S))
    {
      m_props.position -= m_props.direction * velocity;
    }
    if (KeyboardState::getKeyState(KeyCode::D))
    {
      m_props.position += m_props.right * velocity;
    }
    if (KeyboardState::getKeyState(KeyCode::A))
    {
      m_props.position -= m_props.right * velocity;
    }
    if (KeyboardState::getKeyState(KeyCode::Space))
    {
      m_props.position.y += 1.4f * velocity;
    }
    if (KeyboardState::getKeyState(KeyCode::LeftControl))
    {
      m_props.position.y -= 1.4f * velocity;
    } // this is for later when i add jump mechanic

    // m_props.position.y = 0.0f;
    updateCameraVectors();
  }

  void Camera::updateCameraVectors()
  {
    glm::vec3 direction;
    direction.x       = cos(glm::radians(m_props.yaw)) * cos(glm::radians(m_props.pitch));
    direction.y       = sin(glm::radians(m_props.pitch));
    direction.z       = sin(glm::radians(m_props.yaw)) * cos(glm::radians(m_props.pitch));
    m_props.direction = glm::normalize(direction);

    m_props.right     = glm::normalize(glm::cross(m_props.direction, m_props.world_up));
    m_props.camera_up = glm::normalize(glm::cross(m_props.right, m_props.direction));
  }

  void Camera::cameraUniform(unsigned int shader_id, const char* uniform)
  {
    int view_location = glGetUniformLocation(shader_id, uniform);
    glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(getViewMatrix()));
  }

  glm::vec3 Camera::getCameraPos() const
  {
    glm::vec3 pos = glm::vec3(m_props.position.x, m_props.position.y, m_props.position.z);
    return pos;
  }
  CameraProps& Camera::getProps()
  {
    return m_props;
  }

  void Camera::processMouseClicked()
  {
    return;
  }
} // namespace kogayonon