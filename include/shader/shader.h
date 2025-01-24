#pragma once
#include <glad/glad.h>
#include <string>
enum  ShaderType
{
  NONE = 0,
  VERTEX = 1,
  FRAGMENT = 2
};

struct ShaderProgramSource
{
  ShaderProgramSource(std::string& vert, std::string& frag) :vertex_source(vert), fragment_source(frag) {}
  ShaderProgramSource() = default;
  std::string vertex_source;
  std::string fragment_source;
};

class Shader
{
public:

  Shader(const char* shader_path);
  ~Shader() = default;

  ShaderProgramSource parseShaderFile(const std::string& file_path);
  void bind() const;
  void unbind() const;
  GLint getShaderId();

private:
  static unsigned int compileShader(unsigned int shader_type, std::string& source_data);
  static int createShader(ShaderProgramSource& src);

private:
  GLint m_program_id;
  ShaderProgramSource m_shader_src;
};

