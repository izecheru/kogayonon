#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>

namespace kogayonon_resources
{
template <typename T>
struct Keyframe
{
  float time;
  // glm::vec3, glm::quat
  T data;
};

struct NodeAnim
{
  std::vector<Keyframe<glm::vec3>> translation;
  std::vector<Keyframe<glm::quat>> rotation;
  std::vector<Keyframe<glm::vec3>> scale;
};

struct Joint
{
  Joint* parent;
  std::vector<Joint*> children;

  // the id might not be needed, idk what i can use it for
  // it is the index in the joint vector
  std::string name;
  uint32_t id;
  int gltfJointIndex;

  NodeAnim animationData;

  glm::mat4 inverseBind;
  glm::mat4 localMatrix;
  glm::mat4 globalMatrix;
};

struct Skeleton
{
  std::vector<Joint> joints;
};
} // namespace kogayonon_resources
