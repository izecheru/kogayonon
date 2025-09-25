#include "rendering/camera/camera.hpp"
#include <glm/ext/matrix_transform.hpp>

namespace kogayonon_rendering
{
Camera::Camera()
{
  setupCamera();
}

void Camera::setupCamera()
{
  m_props.position = glm::vec3( 0.0f, 0.0f, 3.0f );
  m_props.direction = glm::vec3( 0.0f, 0.0f, -1.0f );
  m_props.cameraUp = glm::vec3( 0.0f, 1.0f, 0.0f );
  m_props.worldUp = glm::vec3( 0.0f, 1.0f, 0.0f );
  m_props.yaw = -90.0f;
  m_props.pitch = 0.0f;
  m_props.mouse_sens = 0.2f;
  m_props.movement_speed = 90.0f;
}

const glm::mat4& Camera::getViewMatrix() const
{
  static glm::mat4 view;
  view = glm::lookAt( m_props.position, m_props.position + m_props.direction, m_props.cameraUp );
  return view;
}

void Camera::processMouseMoved( float x, float y, bool constrain_pitch )
{
  static float lastX = x;
  static float lastY = y;

  if ( first_move )
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
  if ( constrain_pitch )
  {
    if ( m_props.pitch > 89.0f )
      m_props.pitch = 89.0f;
    if ( m_props.pitch < -89.0f )
      m_props.pitch = -89.0f;
  }

  // wrap yaw within [-180, 180]
  if ( m_props.yaw > 180.0f )
    m_props.yaw -= 360.0f;
  if ( m_props.yaw < -180.0f )
    m_props.yaw += 360.0f;

  updateCameraVectors();
}

void Camera::updateCameraVectors()
{
  glm::vec3 direction;
  direction.x = cos( glm::radians( m_props.yaw ) ) * cos( glm::radians( m_props.pitch ) );
  direction.y = sin( glm::radians( m_props.pitch ) );
  direction.z = sin( glm::radians( m_props.yaw ) ) * cos( glm::radians( m_props.pitch ) );
  m_props.direction = glm::normalize( direction );

  m_props.right = glm::normalize( glm::cross( m_props.direction, m_props.worldUp ) );
  m_props.cameraUp = glm::normalize( glm::cross( m_props.right, m_props.direction ) );
}

} // namespace kogayonon_rendering