#pragma once
#include <glm/glm.hpp>
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
  int gltfTargetNode;
  std::vector<Keyframe<glm::vec3>> translations;
  std::vector<Keyframe<glm::quat>> rotation;
  std::vector<Keyframe<glm::vec3>> scale;
};

struct Joint
{
  // the id might not be needed, idk what i can use it for
  // it is the index in the joint vector
  uint32_t id;
  Joint* parent;
  std::string name;
  std::vector<Joint*> children;
  glm::mat4 inverseBind;
  glm::mat4 localMatrix;
  glm::mat4 globalMatrix;
};

struct Skeleton
{
  // each joint has a node index in the gltf file
  // so gltfNodeIndex.at(joints.at(0)) returns the gltf node index
  std::vector<int> gltfNodeIndex;
  std::vector<Joint> joints;

  std::vector<NodeAnim> nodeAnimations;
};
} // namespace kogayonon_resources
