#pragma once
#include <glad/glad.h>
#include <string>
#include <glm\ext\matrix_float4x4.hpp>
#include "core/singleton/singleton.h"

namespace kogayonon
{
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
    ~Shader() = default;
    Shader() = default;

    Shader(const char* vert_path, const char* frag_path);

    ShaderProgramSource parseShaderFile(const std::string& vert_path, const std::string& frag_path);

    void bind() const;
    void unbind() const;

    void setInt(const char* uniform, int value);
    void setMat4(const char* uniform, glm::mat4 mat);

    GLint getShaderId();

  private:
    static unsigned int compileShader(unsigned int shader_type, std::string& source_data);
    static int createShader(ShaderProgramSource& src);

  private:
    GLint m_program_id;
    ShaderProgramSource m_shader_src;
  };
}
