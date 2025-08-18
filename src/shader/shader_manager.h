#pragma once
#include <string.h>

#include "shader.h"

namespace kogayonon
{

  class ShaderManager
  {
  public:
    ShaderManager() {}

    ~ShaderManager() {}

    unsigned int getShaderId(const std::string& shader_name);
    void pushShader(const std::string& vertex_shader, const std::string& fragment_shader, const std::string& shader_name);
    Shader& getShader(const std::string& shader_name);
    void bindShader(const std::string& shader_name);
    void unbindShader(const std::string& shader_name);

  private:
    std::unordered_map<std::string, Shader> m_shaders{};
  };
} // namespace kogayonon