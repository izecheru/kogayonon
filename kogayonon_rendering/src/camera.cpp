#include "rendering/camera/camera.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <spdlog/spdlog.h>

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
  m_props.mouseSensitivity = 0.2f;
  m_props.movementSpeed = 90.0f;
  m_props.mouseZoomSpeed = 1.0f;
}

glm::mat4& Camera::getViewMatrix() const
{
  static glm::mat4 view;
  view = glm::lookAt( m_props.position, m_props.position + m_props.direction, m_props.cameraUp );
  return view;
}

void Camera::onMouseMoved( float x, float y, bool constrainPitch = true )
{
  m_props.yaw += x * m_props.mouseSensitivity;
  m_props.pitch -= y * m_props.mouseSensitivity;

  // this is to no go deaberbeleacu
  if ( constrainPitch )
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

void Camera::zoom( float amount )
{
  m_props.position += m_props.direction * amount * m_props.mouseZoomSpeed;
}

} // namespace kogayonon_rendering