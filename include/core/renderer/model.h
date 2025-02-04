#pragma once
#include <vector>
#include <assimp/scene.h>
#include "shader/shader.h"
#include "core/renderer/mesh.h"

class Model
{
public:
  Model(const char* path_to_model);
  ~Model() = default;

  void draw(Shader& shader);

  void loadModel(const char* path);

private:
  std::vector<Mesh> m_meshes;
  const char* path;
};