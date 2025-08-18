#include "shader_manager.h"

namespace kogayonon
{
  void ShaderManager::pushShader(const std::string& vertex_shader, const std::string& fragment_shader, const std::string& shader_name)
  {
    Shader sh(vertex_shader, fragment_shader);
    m_shaders.emplace(shader_name, sh);
  }

  Shader& ShaderManager::getShader(const std::string& shader_name)
  {
    if (m_shaders.find(shader_name) == m_shaders.end())
    {
      throw std::runtime_error("Shader not found");
    }
    return m_shaders.at(shader_name);
  }

  unsigned int ShaderManager::getShaderId(const std::string& shader_name)
  {
    return getShader(shader_name).getShaderId();
  }

  void ShaderManager::bindShader(const std::string& shader_name)
  {
    getShader(shader_name).bind();
  }

  void ShaderManager::unbindShader(const std::string& shader_name)
  {
    getShader(shader_name).unbind();
  }
} // namespace kogayonon
