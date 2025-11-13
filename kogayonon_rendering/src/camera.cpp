#include "rendering/camera/camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include "utilities/input/keyboard_state.hpp"
using namespace kogayonon_utilities;

namespace kogayonon_rendering
{
Camera::Camera()
{
  setupCamera();
}

void Camera::setupCamera()
{
  m_props.translation = glm::vec3{ 0.0f, 2.0f, 20.0f };
  m_props.direction = glm::vec3{ 0.0f, 0.0f, -1.0f };
  m_props.cameraUp = glm::vec3{ 0.0f, 1.0f, 0.0f };
  m_props.worldUp = glm::vec3{ 0.0f, 1.0f, 0.0f };
  m_props.yaw = -90.0f;
  m_props.pitch = -10.0f;
  m_props.mouseSensitivity = 0.2f;
  m_props.movementSpeed = 90.0f;
  m_props.mouseZoomSpeed = 1.0f;
  m_props.fov = 45.0f;
}

glm::mat4& Camera::getViewMatrix() const
{
  static glm::mat4 view;
  view = glm::lookAt( m_props.translation, m_props.translation + m_props.direction, m_props.cameraUp );
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

void Camera::onKeyPressed( float delta )
{
  float velocity = m_props.movementSpeed * delta;

  if ( KeyboardState::getKeyState( KeyCode::W ) )
  {
    m_props.translation += m_props.direction * velocity;
  }
  if ( KeyboardState::getKeyState( KeyCode::S ) )
  {
    m_props.translation -= m_props.direction * velocity;
  }
  if ( KeyboardState::getKeyState( KeyCode::D ) )
  {
    m_props.translation += glm::normalize( glm::cross( m_props.direction, m_props.cameraUp ) ) * velocity;
  }
  if ( KeyboardState::getKeyState( KeyCode::A ) )
  {
    m_props.translation -= glm::normalize( glm::cross( m_props.direction, m_props.cameraUp ) ) * velocity;
  }
  if ( KeyboardState::getKeyState( KeyCode::Space ) )
  {
    m_props.translation.y += 1.4f * velocity;
  }
  if ( KeyboardState::getKeyState( KeyCode::LeftControl ) )
  {
    m_props.translation.y -= 1.4f * velocity;
  }

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
  if ( m_props.fov >= 1.0f && m_props.fov <= 45.0f )
    m_props.fov -= amount;

  if ( m_props.fov <= 1.0f )
    m_props.fov = 1.0f;
  if ( m_props.fov >= 45.0f )
    m_props.fov = 45.0f;
}

glm::mat4 Camera::getProjectionMatrix( const glm::vec2& contentSize ) const
{
  return glm::perspective( glm::radians( m_props.fov ), contentSize.x / contentSize.y, 0.1f, 1000.0f );
}
} // namespace kogayonon_rendering