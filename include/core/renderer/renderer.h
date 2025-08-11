#pragma once
#include <map>

#include "core/asset_manager/loader/mesh.h"
#include "core/layer/layer_stack.h"
#include "shader/shader.h"

namespace kogayonon
{
  class Renderer
  {
  public:
    Renderer() : is_poly(false) {}
    ~Renderer() = default;

    void draw() const;
    LayerStack& getLayerStack();
    void pushLayer(std::unique_ptr<Layer> layer);
    bool getPolyMode();
    void togglePolyMode();

    // TODO breaking SRP here, move them to shader
    void pushShader(const std::string& vertex_shader, const std::string& fragment_shader, const std::string& shader_name);
    Shader& getShader(const char* shader_name);
    GLint getShaderId(const char* shader_name);
    void bindShader(const char* shader_name);
    void unbindShader(const char* shader_name);

  private:
    bool is_poly = false;
    LayerStack m_layer_stack;
    std::map<const std::string, Shader> m_shaders;
  };
} // namespace kogayonon
