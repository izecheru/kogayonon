#pragma once
#include "core/logger.h"

#include <map>
#include <optional>
#include <shader/shader.h>

class Renderer
{
public:
  Renderer() = default;
  ~Renderer() = default;

  void render(const char* object_name) {}

  void pushShader(const char* shader_file_path, const char* shader_name);

  Shader getShader(const char* shader_name);
  GLint getShaderId(const char* shader_name);

  void bindShader(const char* shader_name);
  void unbindShader(const char* shader_name);

private:
  std::map<const char*, Shader> m_shaders_array;
};

