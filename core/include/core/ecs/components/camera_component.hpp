#pragma once
#include <glm/glm.hpp>

namespace core
{
struct CameraUbo
{
  glm::mat4 view;
  glm::mat4 projection;
};

struct CameraComponent
{
  CameraUbo ubo;
  bool isUsed{ false };
};
} // namespace core