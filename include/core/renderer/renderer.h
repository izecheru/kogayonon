#pragma once
#include <map>
#include "core/asset_manager/model_loader/mesh.h"
#include "core/layer/layer_stack.h"
#include "shader/shader.h"

namespace kogayonon
{
  class Renderer
  {
  public:
    Renderer();
    ~Renderer() = default;

    void draw();

    LayerStack& getLayerStack();

    void pushShader(const std::string& vertex_shader, const std::string& fragment_shader, const std::string& shader_name);
    void pushLayer(std::unique_ptr<Layer> layer);

    Shader& getShader(const char* shader_name);
    GLint getShaderId(const char* shader_name);

    void bindShader(const char* shader_name);
    void unbindShader(const char* shader_name);

    bool getPolyMode();
    void togglePolyMode();

  private:
    bool is_poly;
    LayerStack m_layer_stack;
    std::map<const std::string, Shader> m_shaders;
  };
}
