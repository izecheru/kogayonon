#pragma once
#include <vector>
#include "shader/shader.h"
#include "core/renderer/mesh.h"

class Model
{
public:
  Model(const char* path, std::vector<Mesh>& meshes) :path_to_model(path), m_meshes(meshes) {}
  ~Model() = default;
  Model() = default;

  void draw(Shader& shader);

private:
  std::vector<Mesh> m_meshes;
  const char* path_to_model;
};