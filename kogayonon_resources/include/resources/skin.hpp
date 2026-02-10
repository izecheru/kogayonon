#pragma once
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>

namespace kogayonon_resources
{

struct Joint
{
  Joint* parent{ nullptr };
  std::vector<Joint*> children;

  int id;
  std::string name;
  glm::mat4 inverseBind;
  glm::mat4 localMatrix;
};

struct Skeleton
{
  std::vector<Joint> joints;
};
} // namespace kogayonon_resources
