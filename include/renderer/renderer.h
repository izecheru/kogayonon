#pragma once
#include "core/logger.h"
#include "renderer/vertex_array_buffer.h"
#include "shader/shader.h"
#include <map>

namespace kogayonon
{

  class Renderer {
  public:
    Renderer() = default;
    ~Renderer() = default;

    void render();
    void pushShader(const char* shader_file_path, const char* shader_name);
    Shader getShader(const char* shader_name);
    GLint getShaderId(const char* shader_name);
    void bindShader(const char* shader_name);
    void unbindShader(const char* shader_name);

    void bindShaders();
    void unbindShaders();

  private:
    VertexArrayBuffer m_vao;
    std::map<const char*, Shader> m_shaders_array;
  };

}
