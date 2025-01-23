#include "shader/shader.h"
#include "core/logger.h"

#include <malloc.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

namespace kogayonon
{
  Shader::Shader(const char* shader_path) {
    m_shader_src = parseShaderFile(shader_path);
    m_program_id = createShader(m_shader_src);
  }

  ShaderProgramSource Shader::parseShaderFile(const std::string& file_path) {
    std::ifstream stream(file_path);
    if (!stream.is_open())
    {
      Logger::logError("Failed to open shader file: ", file_path);
      std::string result = "";
      return { result,result };
    }

    std::stringstream ss[3]; // 0 for vertex, 1 for fragment
    std::string line;
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line))
    {
      if (line.find("#shader") != std::string::npos)
      {
        if (line.find("vertex") != std::string::npos)
        {
          type = ShaderType::VERTEX;
        }
        else if (line.find("fragment") != std::string::npos)
        {
          type = ShaderType::FRAGMENT;
        }
      }
      else if (type != ShaderType::NONE)
      {
        ss[(int)type] << line << '\n';
      }
    }
    Logger::logInfo(ss[1].str());
    Logger::logInfo(ss[2].str());
    return { ss[1].str(), ss[2].str() };
  }
  void Shader::bind() const {
    glUseProgram(m_program_id);
  }

  void Shader::unbind() const {
    glUseProgram(0);
  }

  GLint Shader::getShaderId() {
    return m_program_id;
  }

  unsigned int Shader::compileShader(unsigned int shader_type, std::string& source_data) {
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
      char* message = (char*)_malloca(length * sizeof(char));
      glGetShaderInfoLog(id, length, &length, message);
      if (shader_type == GL_VERTEX_SHADER)
      {
        Logger::logError("Failed to compile vertex shader:\n", message, '\n');
      }
      else if (shader_type == GL_FRAGMENT_SHADER)
      {
        Logger::logError("Failed to compile fragment shader:\n", message, '\n');
      }
      glDeleteShader(id);
      return 0;
    }
    Logger::logInfo("Shader compiled successfully:", shader_type == GL_VERTEX_SHADER ? "Vertex Shader" : "Fragment Shader");
    return id;
  }

  int Shader::createShader(ShaderProgramSource& src) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, src.vertex_source);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, src.fragment_source);

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
      char* message = (char*)malloc(length * sizeof(char));
      glGetProgramInfoLog(program, length, &length, message);

      Logger::logError("Failed to link shader program:\n", message, '\n');
      free(message);
      glDeleteProgram(program);
      return 0;
    }
    glValidateProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);

    Logger::logInfo("Succesfully linked shaders", '\n');
    return program;
  }
}

