#pragma once
#include <mutex>

#include "shader.h"

namespace kogayonon
{
class ShaderManager
{
public:
  ShaderManager()
  {
    pushShader("shaders/3d_vertex.glsl", "shaders/3d_fragment.glsl", "3d");
  }

  ~ShaderManager() {}

  unsigned int getShaderId(const std::string& shader_name);
  void pushShader(const std::string& vertex_shader, const std::string& fragment_shader, const std::string& shader_name);
  Shader& getShader(const std::string& shader_name);
  void bindShader(const std::string& shader_name);
  void unbindShader(const std::string& shader_name);

private:
  std::mutex m_mutex;
  std::unordered_map<std::string, Shader> m_shaders{};
};
} // namespace kogayonon