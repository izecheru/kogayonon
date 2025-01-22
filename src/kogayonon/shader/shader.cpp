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

  Shader::ShaderProgramSource Shader::parseShaderFile(const char* file_path) {
    enum class ShaderType {
      NONE = -1,
      VERTEX = 0,
      FRAGMENT = 1
    };

    std::ifstream f(file_path);
    if (!f.is_open()) {
      Logger::logError("Failed to open file", file_path);
      return { "", "" };
    }
    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (std::getline(f, line)) {
      if (line.find("#shader") != std::string::npos) {
        if (line.find("vertex") != std::string::npos) {
          type = ShaderType::VERTEX;
        }
        else if (line.find("fragment") != std::string::npos) {
          type = ShaderType::FRAGMENT;
        }
      }
      else {
        ss[(int)type] << line << '\n';
      }
    }
    std::string vertex_s = ss[0].str();
    std::string fragment_s = ss[1].str();

    const char* vertex = _strdup(vertex_s.c_str());
    const char* fragment = _strdup(fragment_s.c_str());

    //printf("vertex \n%s\nfragment\n%s\n", vertex, fragment);
    Logger::logInfo(vertex);
    Logger::logInfo(fragment);
    return { vertex, fragment };
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

  unsigned int Shader::compileShader(unsigned int shader_type, const char* source_data) {
    unsigned int id = glCreateShader(shader_type);
    glShaderSource(id, 1, &source_data, nullptr);
    glCompileShader(id);
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
      int length;
      glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
      char* message = (char*)_malloca(length * sizeof(char));
      glGetShaderInfoLog(id, length, &length, message);
      if (shader_type == GL_VERTEX_SHADER) {
        Logger::logError("Failed to compile vertex shader:\n", message, '\n');
      }
      else if (shader_type == GL_FRAGMENT_SHADER) {
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
    if (result == GL_FALSE) {
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

