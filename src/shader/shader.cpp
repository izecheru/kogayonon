#include "shader/shader.h"

#include <glad/glad.h>

#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "core/context_manager/context_manager.h"

namespace kogayonon
{
  Shader::Shader(const std::string& vert_path, const std::string& frag_path)
  {
    m_shader_src = parseShaderFile(vert_path, frag_path);
    m_program_id = createShader(m_shader_src);
  }

  shader_source Shader::parseShaderFile(const std::string& vert_path, const std::string& frag_path)
  {
    std::ifstream vertex_stream(vert_path);
    if (!vertex_stream.is_open())
    {
      ContextManager::klogger()->log(LogType::ERROR, "Failed to open shader file: ", vert_path);
      std::string result = "";
      assert(result.size() > 0);
      return {result, result};
    }

    std::stringstream vertex_ss; // 0 for vertex, 1 for fragment
    std::string line;

    while (getline(vertex_stream, line))
    {
      vertex_ss << line << '\n';
    }

    std::ifstream fragment_stream(frag_path);
    if (!fragment_stream.is_open())
    {
      ContextManager::klogger()->log(LogType::ERROR, "Failed to open shader file: ", frag_path);
      std::string result = "";

      assert(result.size() > 0);
      return {result, result};
    }

    std::stringstream fragment_ss; // 0 for vertex, 1 for fragment
    line = "";
    while (getline(fragment_stream, line))
    {
      fragment_ss << line << '\n';
    }
    std::string vertex   = vertex_ss.str();
    std::string fragment = fragment_ss.str();

    shader_source source(vertex, fragment);
    return source;
  }

  void Shader::bind() const
  {
    glUseProgram(m_program_id);
  }

  void Shader::unbind() const
  {
    glUseProgram(0);
  }

  void Shader::setInt(const char* uniform, int value) const
  {
    if (int location = glGetUniformLocation(m_program_id, uniform); location == -1)
    {
      // Uniform not found, print a warning or error message
      ContextManager::klogger()->log(LogType::ERROR, "Uniform not found: ", uniform);
    }
    else
    {
      glUniform1i(location, value); // Set the uniform value
    }
  }

  void Shader::setMat4(const char* uniform, glm::mat4& mat)
  {
    if (int location = glGetUniformLocation(m_program_id, uniform); location == -1)
    {
      ContextManager::klogger()->log(LogType::ERROR, "Uniform not found: ", uniform);
    }
    else
    {
      glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(mat));
    }
  }

  GLint Shader::getShaderId()
  {
    return m_program_id;
  }

  unsigned int Shader::compileShader(unsigned int shader_type, std::string& source_data)
  {
    unsigned int id = glCreateShader(shader_type);
    const char* src = source_data.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
      int length;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
      auto message = (char*)_malloca(length * sizeof(char));
      glGetShaderInfoLog(id, length, &length, message);
      if (shader_type == GL_VERTEX_SHADER)
      {
        ContextManager::klogger()->log(LogType::INFO, "Failed to compile vertex shader:\n", message, '\n');
      }
      else if (shader_type == GL_FRAGMENT_SHADER)
      {
        ContextManager::klogger()->log(LogType::INFO, "Failed to compile fragment shader:\n", message, '\n');
      }
      glDeleteShader(id);
      return 0;
    }
    ContextManager::klogger()->log(LogType::INFO,
                                   "Shader compiled successfully:", shader_type == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader");
    return id;
  }

  int Shader::createShader(shader_source& src)
  {
    unsigned int program = glCreateProgram();
    unsigned int vs      = compileShader(GL_VERTEX_SHADER, src.vertex_source);
    unsigned int fs      = compileShader(GL_FRAGMENT_SHADER, src.fragment_source);

    // Attach shaders now
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    int result;
    glGetProgramiv(program, GL_LINK_STATUS, &result);
    if (result == GL_FALSE)
    {
      int length;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
      auto message = (char*)malloc(length * sizeof(char));
      glGetProgramInfoLog(program, length, &length, message);

      ContextManager::klogger()->log(LogType::INFO, "Failed to link shader program:\n", message, '\n');
      free(message);
      glDeleteProgram(program);
      return 0;
    }
    glValidateProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    ContextManager::klogger()->log(LogType::INFO, "Succesfully linked shaders", '\n');
    return program;
  }
} // namespace kogayonon
