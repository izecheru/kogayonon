#pragma once
#include <glm/glm.hpp>

namespace kogayonon_rendering
{
struct CameraProps
{
  glm::vec3 position;
  glm::vec3 direction;
  glm::vec3 cameraUp;
  glm::vec3 worldUp;
  glm::vec3 right;
  glm::mat4x4 view;
  float yaw;
  float pitch;
  float movement_speed;
  float zoom;
  float mouse_sens;
};

class Camera
{
public:
  Camera();
  ~Camera() = default;

  void setupCamera();

  const glm::mat4& getViewMatrix() const;
  void processMouseMoved( float x, float y, bool constrain_pitch );
  void updateCameraVectors();
  void cameraUniform( unsigned int shader_id, const char* uniform );

  inline float getX() const
  {
    return m_props.position.x;
  }

  inline float getY() const
  {
    return m_props.position.y;
  }

  inline float getZ() const
  {
    return m_props.position.z;
  }

  inline glm::vec3& getPosition()
  {
    return m_props.position;
  }

  inline CameraProps& getProps()
  {
    return m_props;
  }

private:
  CameraProps m_props;
  bool first_move = false;
};
} // namespace kogayonon_rendering