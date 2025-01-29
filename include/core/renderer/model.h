#pragma once
#include <vector>
#include "shader/shader.h"
#include "core/renderer/mesh.h"


class Model
{
public:
  Model(const char* path);
  Model() {}
  ~Model() = default;

  void draw(Shader& shader, Camera& camera);
private:
  std::vector<unsigned char> m_data;
  std::vector<Mesh> m_meshes;
  const char* m_file;
};