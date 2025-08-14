#pragma once
#include <glm/glm.hpp>

#include "core/singleton/singleton.h"
#include "event/keyboard_events.h"
#include "event/mouse_events.h"

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

  class Camera : public Singleton<Camera>
  {
  public:
    Camera();

    void setupCamera();

    const CameraProps& getCamera();
    const bool getFirstMove();

    void setFirstMove();
    const glm::mat4& getViewMatrix() const;

    bool onMouseScrolled(MouseScrolledEvent& event);
    void processMouseScrolled(double x_offset, double y_offset);

    bool onMouseMoved(MouseMovedEvent& event);
    void processMouseMoved(float x, float y, bool constrain_pitch = true);

    bool onMouseClicked(MouseClickedEvent& event);
    void processMouseClicked();

    void processKeyboard();
    void updateCameraVectors();

    void cameraUniform(unsigned int shader_id, const char* uniform);
    glm::vec3 getCameraPos() const;

    CameraProps& getProps();

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

  private:
    CameraProps m_props{};
    bool first_move = false;
  };
} // namespace kogayonon