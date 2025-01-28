#pragma once
#include "core/singleton/singleton.h"
#include <glm/ext/matrix_float4x4.hpp>
#include "core/key_codes.h"
#include <GLFW/glfw3.h>

// https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h
struct CameraProps
{
  glm::vec3 position;
  glm::vec3 direction;
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
  void processKeyboard(GLFWwindow* window, float delta_time);

  void updateCameraVectors();

private:
  CameraProps m_props;
  bool first_move = false;
};