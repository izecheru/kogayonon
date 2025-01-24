#pragma once
#include <core/singleton/singleton.h>
#include <glm/matrix.hpp>

// https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h
enum CameraMovement
{
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT
};

// Default camera values
#define YAW  -90.0f;
#define PITCH  0.0f;
#define SPEED  2.5f;
#define SENSITIVITY  0.1f;
#define ZOOM  45.0f;

struct CameraProps
{
  glm::vec3 camera_pos;
  glm::vec3 camera_front;
  glm::vec3 camera_up;
  glm::vec3 world_up;
  glm::vec3 right;
  glm::mat4x4 view;
  float yaw;
  float pitch;
  float movement_speed;
  float zoom;
  float mouse_sens;
};

class Camera :public Singleton<Camera>
{
public:
  //friend class Singleton<Camera>;
  Camera();

  void updateView();
  void setView();

  const CameraProps& getCamera();
  const bool getFirstMove();

  void setFirstMove();
  const glm::mat4& getViewMatrix()const;

  void processMouseMoved(float x, float y, bool constrain_pitch = true);

  void updateCameraVectors();

private:
  // kind of ugly but i think it gets the job done for the moment
  CameraProps m_props;
  bool first_move = false;
};