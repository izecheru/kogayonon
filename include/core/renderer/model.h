#pragma once
#include <vector>
#include "shader/shader.h"
#include "core/renderer/mesh.h"


class Model
{
public:
  Model(const char* path);
  ~Model() = default;
  Model() {}

  void draw(Shader& shader);

private:
  std::vector<Mesh> m_meshes;
};