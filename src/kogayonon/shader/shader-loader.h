#pragma once
#include <glad/glad.h>
// TODO: write shaders in one file and separate them with code
class Shader {
public:
  struct ShaderProgramSource {
    const char* vertex_source;
    const char* fragment_source;
  };
  ShaderProgramSource parseShaderFile(const char* file_path);
  static unsigned int compileShader(unsigned int shader_type, const char* source_data);
  static int createShader(const char* vertex_shader_data, const char* fragment_shader_data);
};
