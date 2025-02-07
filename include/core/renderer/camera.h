#pragma once
#include "core/singleton/singleton.h"
#include <glm/glm.hpp>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace kogayonon
{

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

    void setView();

    const CameraProps& getCamera();
    const bool getFirstMove();

    void setFirstMove();
    const glm::mat4& getViewMatrix()const;

    void processMouseMoved(float x, float y, bool constrain_pitch = true);
    void processKeyboard(GLFWwindow* window, float delta_time);

    void updateCameraVectors();

    void cameraUniform(unsigned int shader_id, const char* uniform);
    glm::vec3 getCameraPos()const;

  private:
    CameraProps m_props;
    bool first_move = false;
  };
}