#pragma once
#include <glad/glad.h>
#include <string>
#include <sstream>
#include <iostream>

namespace kogayonon
{
  class Logger
  {
  public:
    Logger() = delete;

  private:
    static void red();
    static void green();
    static void reset();

    template <typename T>
    static void appendToStream(std::stringstream& stream, const T& value) {
      stream << value;
    }

    template <typename T, typename... Args>
    static void appendToStream(std::stringstream& stream, const T& value, const Args&... args) {
      stream << value;
      appendToStream(stream, args...);
    }

  public:
    template <typename... Args>
    static void logError(const Args&... args) {
      red();
      std::stringstream stream;
      appendToStream(stream, args...);
      std::cerr << "[ERROR]: " << stream.str() << std::endl;
      reset();
    }


    static std::string getOpenGLErrorString(GLenum error) {
      switch (error)
      {
        case GL_NO_ERROR:          return "GL_NO_ERROR";
        case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
        case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
        case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default:                   return "Unknown OpenGL Error"; // Handle unknown errors
      }
    }

    static bool logOpenGLErr(const std::string& file, const int line) {
      red();
      auto error = glGetError();
      if (error)
      {
        std::cerr << "[OPENGL_ERR " << getOpenGLErrorString(error) << " in file " << file << "," << line << "] " << std::endl;
        reset();
        return true;
      }
      return false;
    }

    template <typename... Args>
    static void logInfo(const Args&... args) {
      green();
      std::stringstream stream;
      appendToStream(stream, args...);
      std::cout << "[INFO]: " << stream.str() << std::endl;
      reset();
    }
  };
}
