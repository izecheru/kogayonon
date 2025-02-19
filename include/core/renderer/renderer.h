#pragma once
#include <map>
#include "core/model_loader/mesh.h"
#include "core/layer/layer_stack.h"
#include "shader/shader.h"

namespace kogayonon
{
  class Renderer {
  public:
    Renderer();
    ~Renderer() = default;

    void draw();

    LayerStack& getLayerStack();

    void pushShader(std::string& vertex_shader, std::string& fragment_shader, const char* shader_name);
    void pushLayer(Layer* layer);

    Shader getShader(const char* shader_name);
    GLint getShaderId(const char* shader_name);

    void bindShader(const char* shader_name);
    void unbindShader(const char* shader_name);

    bool getPolyMode();
    void togglePolyMode();

  private:
    bool is_poly;
    LayerStack m_layer_stack;
    std::map<const char*, Shader> m_shaders;
  };
}
