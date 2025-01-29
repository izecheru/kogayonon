#include "core/renderer/renderer.h"

Renderer::Renderer() {
  is_poly = false;
}

//void Renderer::render(const char* mesh_name) {
//  meshes[mesh_name].draw();
//}

void Renderer::pushShader(const char* vertex_shader, const char* fragment_shader, const char* shader_name) {
  Shader sh(vertex_shader, fragment_shader);
  shaders.insert(std::pair<const char*, Shader>(shader_name, sh));
}

void Renderer::pushMesh(const char* mesh_name, const Mesh& mesh) {
  meshes[mesh_name] = mesh;
}

Shader Renderer::getShader(const char* shader_name) {
  for (auto it = shaders.begin(); it != shaders.end(); it++)
  {
    if (it->first == shader_name)
    {
      return it->second;
    }
  }
}

GLint Renderer::getShaderId(const char* shader_name) {
  for (auto it = shaders.begin(); it != shaders.end(); it++)
  {
    if (it->first == shader_name)
    {
      return it->second.getShaderId();
    }
  }
}

void Renderer::bindShader(const char* shader_name) {
  for (auto it = shaders.begin(); it != shaders.end(); it++)
  {
    if (it->first == shader_name)
    {
      it->second.bind();
    }
  }
}

void Renderer::unbindShader(const char* shader_name) {
  for (auto it = shaders.begin(); it != shaders.end(); it++)
  {
    if (it->first == shader_name)
    {
      it->second.unbind();
    }
  }
}

bool Renderer::getPolyMode() {
  return is_poly;
}

void Renderer::togglePolyMode() {
  if (is_poly)
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
  else
  {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  is_poly = !is_poly;
}

