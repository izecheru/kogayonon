#pragma once
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace kogayonon_resources
{

struct Joint
{
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
  // for the joint at joints.at(0), we store the cgltf_node index in the gltfNodeIndex vector
  std::vector<int> gltfNodeIndex;
  std::vector<Joint> joints;
};
} // namespace kogayonon_resources
