#include "core/renderer/renderer.h"

namespace kogayonon
{
  Renderer::Renderer()
  {
    is_poly = false;
  }

  void Renderer::draw()const
  {
    m_layer_stack.draw();
  }

  LayerStack& kogayonon::Renderer::getLayerStack()
  {
    return m_layer_stack;
  }

  void Renderer::pushShader(const std::string& vertex_shader, const std::string& fragment_shader, const std::string& shader_name)
  {
    Shader sh(vertex_shader, fragment_shader);
    m_shaders.try_emplace(shader_name, sh);
  }

  void Renderer::pushLayer(std::unique_ptr<Layer> layer)
  {
    m_layer_stack.pushLayer(std::move(layer));
  }

  Shader& Renderer::getShader(const char* shader_name)
  {
    return m_shaders[shader_name];
  }

  GLint Renderer::getShaderId(const char* shader_name)
  {
    return m_shaders[shader_name].getShaderId();
  }

  void Renderer::bindShader(const char* shader_name)
  {
    m_shaders[shader_name].bind();
  }

  void Renderer::unbindShader(const char* shader_name)
  {
    m_shaders[shader_name].unbind();
  }

  bool Renderer::getPolyMode()
  {
    return is_poly;
  }

  void Renderer::togglePolyMode()
  {
    switch(is_poly)
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
}