#pragma once
#include "core/singleton/singleton.h"
#include <glm/glm.hpp>
#include "events/mouse_events.h"
#include "events/keyboard_events.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace kogayonon
{
  // https://learnopengl.com/code_viewer_gh.php?code=includes/learnopengl/camera.h
  struct CameraProps {
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

  // TODO camera snaps when i leave imgui window
  class Camera :public Singleton<Camera> {
  public:
    Camera();

    void setView();

    const CameraProps& getCamera();
    const bool getFirstMove();

    void setFirstMove();
    const glm::mat4& getViewMatrix()const;

    bool onMouseMoved(MouseMovedEvent& event);

    void processMouseMoved(float x, float y, bool constrain_pitch = true);
    void processKeyboard(double delta_time);

    void updateCameraVectors();

    void cameraUniform(unsigned int shader_id, const char* uniform);
    glm::vec3 getCameraPos()const;

  private:
    CameraProps m_props;
    bool first_move = false;
    unsigned int m_camera_listener_id = 2;
  };
}