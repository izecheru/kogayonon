#include "renderer/renderer.h"

void Renderer::pushShader(const char* shader_file_path, const char* shader_name) {
  Shader sh(shader_file_path);
  m_shaders_array.insert(std::pair<const char*, Shader>(shader_name, sh));
}

Shader Renderer::getShader(const char* shader_name) {
  for (auto it = m_shaders_array.begin(); it != m_shaders_array.end(); it++)
  {
    if (it->first == shader_name)
    {
      return it->second;
    }
  }
  Logger::logError("Shader does not exist");
}

GLint Renderer::getShaderId(const char* shader_name) {
  for (auto it = m_shaders_array.begin(); it != m_shaders_array.end(); it++)
  {
    if (it->first == shader_name)
    {
      return it->second.getShaderId();
    }
  }
}

void Renderer::bindShader(const char* shader_name) {
  for (auto it = m_shaders_array.begin(); it != m_shaders_array.end(); it++)
  {
    if (it->first == shader_name)
    {
      it->second.bind();
    }
  }
}

void Renderer::unbindShader(const char* shader_name) {
  for (auto it = m_shaders_array.begin(); it != m_shaders_array.end(); it++)
  {
    if (it->first == shader_name)
    {
      it->second.unbind();
    }
  }
}

