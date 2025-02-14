#include "core/renderer/renderer.h"

using namespace kogayonon;

Renderer::Renderer() {
  is_poly = false;
}

void Renderer::render() {
  m_layer_stack.render();
}

LayerStack& kogayonon::Renderer::getLayerStack() {
  return m_layer_stack;
}

void Renderer::pushShader(const char* vertex_shader, const char* fragment_shader, const char* shader_name) {
  Shader sh(vertex_shader, fragment_shader);
  m_shaders.insert(std::pair<const char*, Shader>(shader_name, sh));
}

void kogayonon::Renderer::pushLayer(std::unique_ptr<Layer> layer) {
  m_layer_stack.pushLayer(std::move(layer));
}

Shader Renderer::getShader(const char* shader_name) {
  return m_shaders[shader_name];
}

GLint Renderer::getShaderId(const char* shader_name) {
  return m_shaders[shader_name].getShaderId();
}

void Renderer::bindShader(const char* shader_name) {
  m_shaders[shader_name].bind();
}

void Renderer::unbindShader(const char* shader_name) {
  m_shaders[shader_name].unbind();
}

bool Renderer::getPolyMode() {
  return is_poly;
}

void Renderer::togglePolyMode() {
  switch (is_poly)
  {
    case true:
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      break;

    case false:
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      break;
  }

  is_poly = !is_poly;
}
