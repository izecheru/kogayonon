#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace core
{
struct CameraProps
{
  glm::vec3 eye{ 0.0f, 0.0f, 10.0f };
  glm::vec3 center{ 0.0f, 0.0f, 0.0f };
  glm::vec3 up{ 0.0f, 1.0f, 0.0f };
  glm::vec2 extent;
  float fov{ 45.0f };
  float nearView{ 0.01f };
  float farView{ 1000.0f };

  // If we modify the props, update the projection and the view
  bool changed{ false };
};

struct CameraUbo
{
  glm::mat4 view;
  glm::mat4 projection;
};

struct CameraComponent
{
  CameraUbo ubo;
  CameraProps props;
  bool isUsed{ false };

  inline void updateUbo()
  {
    if ( props.changed )
    {
      ubo.view = glm::lookAt( props.eye, props.center, props.up );
      ubo.projection = glm::perspective(
        glm::radians( props.fov ), props.extent.x / (float)props.extent.y, props.nearView, props.farView );
      ubo.projection[1][1] *= -1;
      props.changed = false;
    }
  }
};
} // namespace core