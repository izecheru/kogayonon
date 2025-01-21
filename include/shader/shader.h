#pragma once
#include <glad/glad.h>
namespace kogayonon
{

  class Shader
  {
  public:
    struct ShaderProgramSource
    {
      const char* vertex_source;
      const char* fragment_source;
    };

    Shader(const char* shader_path);
    ~Shader() = default;

    ShaderProgramSource parseShaderFile(const char* file_path);
    void bind() const;
    void unbind() const;

  private:
    static unsigned int compileShader(unsigned int shader_type, const char* source_data);
    static int createShader(ShaderProgramSource& src);

  private:
    GLint m_program_id;
    ShaderProgramSource m_shader_src;
  };

}
