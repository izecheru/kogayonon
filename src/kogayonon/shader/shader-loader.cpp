#include "shader-loader.h"
#include <malloc.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

Shader::ShaderProgramSource Shader::parseShaderFile(const char* file_path) {
  enum class ShaderType {
    NONE = -1,
    VERTEX = 0,
    FRAGMENT = 1
  };

  std::ifstream f(file_path);
  if (!f.is_open()) {
    std::cerr << "Failed to open the shader file " << file_path << '\n';
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
      ss[(int) type] << line << '\n';
    }
  }
  std::string vertex_s = ss[0].str();
  std::string fragment_s = ss[1].str();
  const char* vertex = _strdup(vertex_s.c_str());
  const char* fragment = _strdup(fragment_s.c_str());
  printf("vertex \n%s\nfragment\n%s\n", vertex, fragment);
  return { vertex, fragment };
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
    char* message = (char*) alloca(length * sizeof(char));
    glGetShaderInfoLog(id, length, &length, message);
    if (shader_type == GL_VERTEX_SHADER) {
      std::cout << "failed to compile vertex shader\n";
      std::cout << message << '\n';
    }
    else if (shader_type == GL_FRAGMENT_SHADER) {
      std::cout << "failed to compile fragment shader\n";
      std::cout << message << '\n';
    }
    glDeleteShader(id);
    return 0;
  }
  return id;
}

int Shader::createShader(const char* vertex_shader_data, const char* fragment_shader_data) {
  unsigned int program = glCreateProgram();
  unsigned int vs = compileShader(GL_VERTEX_SHADER, vertex_shader_data);
  unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragment_shader_data);

  // Attach shaders now
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glValidateProgram(program);
  glDeleteShader(vs);
  glDeleteShader(fs);
  return program;
}
