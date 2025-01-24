#include "renderer/renderer.h"

void Renderer::render(const char* mesh_name) {
  m_mesh_array[mesh_name].draw();
}

void Renderer::pushShader(const char* vertex_shader, const char* fragment_shader, const char* shader_name) {
  Shader sh(vertex_shader, fragment_shader);
  m_shaders_array.insert(std::pair<const char*, Shader>(shader_name, sh));
}

void Renderer::pushMesh(const char* mesh_name, const Mesh& mesh) {
  m_mesh_array[mesh_name] = mesh;
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

