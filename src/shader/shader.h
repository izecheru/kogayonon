#pragma once
#include <glad/glad.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <string>

#include "core/singleton/singleton.h"

namespace kogayonon
{
  enum class ShaderType
  {
    NONE     = 0,
    VERTEX   = 1,
    FRAGMENT = 2
  };

  struct shader_source
  {
    shader_source(const std::string& vert, const std::string& frag) : vertex_source(vert), fragment_source(frag) {}
    shader_source() = default;
    std::string vertex_source;
    std::string fragment_source;
  };

  class Shader
  {
  public:
    Shader() = default;

    explicit Shader(const std::string& vert_path, const std::string& frag_path);

    shader_source parseShaderFile(const std::string& vert_path, const std::string& frag_path);

    void bind() const;
    void unbind() const;

    void setInt(const char* uniform, int value) const;
    void setMat4(const char* uniform, glm::mat4& mat);

    GLint getShaderId();

  private:
    static unsigned int compileShader(unsigned int shader_type, std::string& source_data);
    static int createShader(shader_source& src);

  private:
    uint32_t m_program_id = 0;
    shader_source m_shader_src;
  };
} // namespace kogayonon
