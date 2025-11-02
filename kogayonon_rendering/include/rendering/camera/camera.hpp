#pragma once
#include <glm/glm.hpp>

namespace kogayonon_rendering
{
struct CameraProps
{
  glm::vec3 translation;
  glm::vec3 direction;
  glm::vec3 cameraUp;
  glm::vec3 worldUp;
  glm::vec3 right;
  glm::mat4x4 view;
  glm::mat4 projection;
  float fov;
  float yaw;
  float pitch;
  float movementSpeed;
  float zoom;
  float mouseSensitivity;
  float mouseZoomSpeed;
};

class Camera
{
public:
  Camera();
  ~Camera() = default;

  void setupCamera();

  glm::mat4& getViewMatrix() const;
  void onMouseMoved( float x, float y, bool constrainPitch );
  void onKeyPressed( float delta );
  void updateCameraVectors();
  void zoom( float amount );

  inline float getX() const
  {
    return m_props.translation.x;
  }

  inline float getY() const
  {
    return m_props.translation.y;
  }

  inline float getZ() const
  {
    return m_props.translation.z;
  }

  inline glm::vec3& getPosition()
  {
    return m_props.translation;
  }

  inline CameraProps& getProps()
  {
    return m_props;
  }

  glm::mat4 getProjectionMatrix( const glm::vec2& contentSize ) const;

private:
  CameraProps m_props;
};
} // namespace kogayonon_rendering